#include <cmath>
#include <functional>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "map_generator.hpp"
#include "perlin_noise.hpp"
#include "renderers.hpp"
#include "terrain_factory.hpp"
#include "world_map.hpp"

namespace {

int failures = 0;
int checks = 0;

void check(bool condition, const char* expression, const char* file, int line) {
    ++checks;
    if (!condition) {
        ++failures;
        std::cerr << file << ":" << line << ": ПРОВАЛ: " << expression << "\n";
    }
}

#define CHECK(expression) check((expression), #expression, __FILE__, __LINE__)

template <typename Exception, typename Callable>
bool throwsException(Callable callable) {
    try {
        callable();
    } catch (const Exception&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

#define CHECK_THROWS(Exception, expression) check(throwsException<Exception>([&] { expression; }), #expression, __FILE__, __LINE__)

struct TestCase {
    std::string name;
    std::function<void()> body;
};

std::vector<TestCase>& registry() {
    static std::vector<TestCase> tests;
    return tests;
}

struct Registrar {
    Registrar(const std::string& name, std::function<void()> body) { registry().push_back({name, std::move(body)}); }
};

#define TEST(name)                                          \
    void name();                                            \
    const Registrar registrar_##name(#name, name);          \
    void name()

std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        lines.push_back(line);
    }
    return lines;
}

wg::WorldMap makeMap(unsigned seed, int width, int height) {
    return wg::MapGeneratorBuilder().width(width).height(height).seed(seed).build().generate();
}

std::string renderAscii(const wg::WorldMap& map) {
    return wg::AsciiRenderer().render(map);
}

TEST(perlinNoiseIsZeroAtLatticePoints) {
    const wg::PerlinNoise noise(1);
    for (int x = -3; x <= 3; ++x) {
        for (int y = -3; y <= 3; ++y) {
            CHECK(std::abs(noise.noise(x, y)) < 1e-12);
        }
    }
}

TEST(perlinNoiseStaysInRange) {
    const wg::PerlinNoise noise(42);
    bool inRange = true;
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 200; ++j) {
            const double value = noise.noise(i * 0.137, j * 0.173);
            inRange = inRange && value >= -1.0 && value <= 1.0;
            const double octave = noise.octaveNoise(i * 0.137, j * 0.173, 5, 0.5);
            inRange = inRange && octave >= -1.0 && octave <= 1.0;
        }
    }
    CHECK(inRange);
}

TEST(perlinNoiseIsDeterministic) {
    const wg::PerlinNoise first(5);
    const wg::PerlinNoise second(5);
    const wg::PerlinNoise other(6);
    bool same = true;
    bool differs = false;
    for (int i = 0; i < 50; ++i) {
        const double x = i * 0.31 + 0.1;
        const double y = i * 0.17 + 0.2;
        same = same && first.noise(x, y) == second.noise(x, y);
        differs = differs || first.noise(x, y) != other.noise(x, y);
    }
    CHECK(same);
    CHECK(differs);
}

TEST(perlinNoiseIsContinuous) {
    const wg::PerlinNoise noise(9);
    double largestJump = 0.0;
    for (int i = 0; i < 500; ++i) {
        const double x = i * 0.0731;
        const double y = i * 0.0419;
        largestJump = std::max(largestJump, std::abs(noise.noise(x + 1e-4, y) - noise.noise(x, y)));
    }
    CHECK(largestJump < 1e-3);
}

TEST(perlinNoiseHandlesNegativeCoordinates) {
    const wg::PerlinNoise noise(3);
    const double value = noise.noise(-17.35, -250.8);
    CHECK(value >= -1.0 && value <= 1.0);
}

TEST(octaveNoiseWithOneOctaveEqualsBasicNoise) {
    const wg::PerlinNoise noise(11);
    CHECK(noise.octaveNoise(1.3, 2.7, 1, 0.5) == noise.noise(1.3, 2.7));
}

TEST(terrainClassesOverrideVirtualMethods) {
    const std::vector<const wg::Terrain*>& terrains = wg::TerrainFactory::all();
    CHECK(terrains.size() == 7);
    std::set<char> symbols;
    std::set<std::string> names;
    for (const wg::Terrain* terrain : terrains) {
        symbols.insert(terrain->symbol());
        names.insert(terrain->name());
        CHECK(terrain->colorCode() > 0);
    }
    CHECK(symbols.size() == 7);
    CHECK(names.size() == 7);
}

TEST(terrainWalkabilityIsPolymorphic) {
    CHECK(!wg::TerrainFactory::create(0.0, 0.5).isWalkable());
    CHECK(wg::TerrainFactory::create(0.38, 0.5).isWalkable());
    CHECK(wg::TerrainFactory::create(0.5, 0.1).isWalkable());
    CHECK(!wg::TerrainFactory::create(0.7, 0.5).isWalkable());
    CHECK(!wg::TerrainFactory::create(0.95, 0.5).isWalkable());
}

TEST(factoryChoosesTerrainByElevationAndMoisture) {
    CHECK(wg::TerrainFactory::create(0.10, 0.9).symbol() == '~');
    CHECK(wg::TerrainFactory::create(0.38, 0.9).symbol() == '.');
    CHECK(wg::TerrainFactory::create(0.50, 0.10).symbol() == ':');
    CHECK(wg::TerrainFactory::create(0.50, 0.50).symbol() == '"');
    CHECK(wg::TerrainFactory::create(0.50, 0.90).symbol() == '*');
    CHECK(wg::TerrainFactory::create(0.75, 0.50).symbol() == '#');
    CHECK(wg::TerrainFactory::create(0.95, 0.50).symbol() == '^');
}

TEST(factoryReturnsSharedInstances) {
    CHECK(&wg::TerrainFactory::create(0.1, 0.1) == &wg::TerrainFactory::create(0.2, 0.9));
    CHECK(&wg::TerrainFactory::create(0.5, 0.1) != &wg::TerrainFactory::create(0.5, 0.9));
}

TEST(factoryFindsTerrainBySymbol) {
    for (const wg::Terrain* terrain : wg::TerrainFactory::all()) {
        CHECK(wg::TerrainFactory::bySymbol(terrain->symbol()) == terrain);
    }
    CHECK(wg::TerrainFactory::bySymbol('x') == nullptr);
    CHECK(wg::TerrainFactory::bySymbol('+') == nullptr);
}

TEST(worldMapStoresAndReturnsCells) {
    wg::WorldMap map(3, 2, wg::TerrainFactory::create(0.0, 0.0));
    CHECK(map.width() == 3);
    CHECK(map.height() == 2);
    CHECK(map.at(2, 1).symbol() == '~');
    map.set(2, 1, wg::TerrainFactory::create(0.95, 0.0));
    CHECK(map.at(2, 1).symbol() == '^');
    CHECK(map.at(0, 0).symbol() == '~');
}

TEST(worldMapChecksBounds) {
    wg::WorldMap map(3, 2, wg::TerrainFactory::create(0.0, 0.0));
    CHECK_THROWS(std::out_of_range, map.at(3, 0));
    CHECK_THROWS(std::out_of_range, map.at(0, 2));
    CHECK_THROWS(std::out_of_range, map.at(-1, 0));
    CHECK_THROWS(std::out_of_range, map.set(0, -1, wg::TerrainFactory::create(0.0, 0.0)));
    CHECK_THROWS(std::invalid_argument, wg::WorldMap(0, 5, wg::TerrainFactory::create(0.0, 0.0)));
    CHECK_THROWS(std::invalid_argument, wg::WorldMap(5, -1, wg::TerrainFactory::create(0.0, 0.0)));
}

TEST(worldMapStatisticsAreConsistent) {
    const wg::WorldMap map = makeMap(3, 40, 20);
    int total = 0;
    double percent = 0.0;
    for (const wg::TerrainStat& stat : map.statistics()) {
        total += stat.count;
        percent += stat.percent;
    }
    CHECK(total == 800);
    CHECK(std::abs(percent - 100.0) < 1e-9);
    CHECK(map.walkablePercent() >= 0.0 && map.walkablePercent() <= 100.0);
}

TEST(worldMapWalkablePercentOnUniformMaps) {
    const wg::WorldMap sea(4, 4, wg::TerrainFactory::create(0.0, 0.0));
    const wg::WorldMap plains(4, 4, wg::TerrainFactory::create(0.5, 0.5));
    CHECK(sea.walkablePercent() == 0.0);
    CHECK(plains.walkablePercent() == 100.0);
}

TEST(builderUsesDefaultsAndFluentSetters) {
    wg::MapGeneratorBuilder builder;
    CHECK(&builder.width(30) == &builder);
    const wg::MapGenerator generator = builder.height(10).seed(99).scale(12.5).octaves(3).persistence(0.4).aspect(1.5).build();
    CHECK(generator.config().width == 30);
    CHECK(generator.config().height == 10);
    CHECK(generator.config().seed == 99);
    CHECK(generator.config().scale == 12.5);
    CHECK(generator.config().octaves == 3);
    CHECK(generator.config().persistence == 0.4);
    CHECK(generator.config().aspect == 1.5);
    CHECK(wg::MapGeneratorBuilder().build().config().width == 60);
}

TEST(builderRejectsInvalidParameters) {
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().width(0).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().width(301).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().height(0).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().height(201).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().scale(0.5).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().octaves(0).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().octaves(9).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().persistence(0.0).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().persistence(1.5).build());
    CHECK_THROWS(std::invalid_argument, wg::MapGeneratorBuilder().aspect(0.0).build());
}

TEST(builderAcceptsBoundaryValues) {
    CHECK(wg::MapGeneratorBuilder().width(1).height(1).build().generate().width() == 1);
    CHECK(wg::MapGeneratorBuilder().width(300).height(200).octaves(8).persistence(1.0).build().config().width == 300);
}

TEST(generatorProducesRequestedSize) {
    const wg::WorldMap map = makeMap(1, 37, 11);
    CHECK(map.width() == 37);
    CHECK(map.height() == 11);
}

TEST(generatorIsDeterministicForSeed) {
    CHECK(renderAscii(makeMap(8, 50, 25)) == renderAscii(makeMap(8, 50, 25)));
    CHECK(renderAscii(makeMap(8, 50, 25)) != renderAscii(makeMap(9, 50, 25)));
}

TEST(generatorHandlesSingleCellMap) {
    const wg::WorldMap map = makeMap(1, 1, 1);
    CHECK(map.width() == 1 && map.height() == 1);
    CHECK(renderAscii(map).size() == 2);
}

TEST(largeMapContainsAllTerrainTypes) {
    std::set<char> seen;
    const wg::WorldMap map = makeMap(7, 160, 80);
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            seen.insert(map.at(x, y).symbol());
        }
    }
    CHECK(seen.size() == 7);
}

TEST(asciiRendererMatchesMapSize) {
    const wg::WorldMap map = makeMap(2, 20, 7);
    const std::vector<std::string> lines = splitLines(renderAscii(map));
    CHECK(lines.size() == 7);
    for (const std::string& line : lines) {
        CHECK(line.size() == 20);
    }
}

TEST(frameDecoratorAddsBorder) {
    const wg::WorldMap map = makeMap(2, 20, 7);
    wg::FrameDecorator renderer(std::make_unique<wg::AsciiRenderer>());
    const std::vector<std::string> lines = splitLines(renderer.render(map));
    CHECK(lines.size() == 9);
    CHECK(lines.front() == "+" + std::string(20, '-') + "+");
    CHECK(lines.back() == lines.front());
    CHECK(lines[1].front() == '|' && lines[1].back() == '|' && lines[1].size() == 22);
}

TEST(colorDecoratorOnlyAddsEscapeCodes) {
    const wg::WorldMap map = makeMap(2, 20, 7);
    wg::ColorDecorator renderer(std::make_unique<wg::AsciiRenderer>());
    const std::string colored = renderer.render(map);
    CHECK(colored.find("\033[") != std::string::npos);
    CHECK(wg::stripAnsi(colored) == renderAscii(map));
}

TEST(stripAnsiKeepsPlainTextAndBrokenSequences) {
    CHECK(wg::stripAnsi("abc") == "abc");
    CHECK(wg::stripAnsi("\033[31mred\033[0m") == "red");
    CHECK(wg::stripAnsi("x\033[") == "x");
}

TEST(legendDecoratorListsEveryTerrain) {
    const wg::WorldMap map = makeMap(4, 30, 10);
    wg::LegendDecorator renderer(std::make_unique<wg::AsciiRenderer>(), false);
    const std::string text = renderer.render(map);
    CHECK(text.find("Легенда:") != std::string::npos);
    for (const wg::Terrain* terrain : wg::TerrainFactory::all()) {
        CHECK(text.find(terrain->name()) != std::string::npos);
    }
    CHECK(text.find("Проходимая территория:") != std::string::npos);
}

TEST(coloredLegendContainsEscapeCodes) {
    const wg::WorldMap map = makeMap(4, 30, 10);
    wg::LegendDecorator renderer(std::make_unique<wg::AsciiRenderer>(), true);
    CHECK(renderer.render(map).find("\033[") != std::string::npos);
}

TEST(decoratorsComposeThroughBasePointer) {
    const wg::WorldMap map = makeMap(5, 25, 9);
    std::unique_ptr<wg::IMapRenderer> renderer = std::make_unique<wg::AsciiRenderer>();
    renderer = std::make_unique<wg::FrameDecorator>(std::move(renderer));
    renderer = std::make_unique<wg::ColorDecorator>(std::move(renderer));
    renderer = std::make_unique<wg::LegendDecorator>(std::move(renderer), true);
    const std::string plain = wg::stripAnsi(renderer->render(map));
    CHECK(plain.find("+-------------------------+") == 0);
    CHECK(plain.find("Легенда:") != std::string::npos);
}

TEST(decoratorRejectsMissingInnerRenderer) {
    CHECK_THROWS(std::invalid_argument, wg::FrameDecorator(nullptr));
    CHECK_THROWS(std::invalid_argument, wg::ColorDecorator(nullptr));
    CHECK_THROWS(std::invalid_argument, wg::LegendDecorator(nullptr, false));
}

}  // namespace

int main() {
    for (const TestCase& test : registry()) {
        const int before = failures;
        test.body();
        std::cout << (failures == before ? "[ OK ] " : "[FAIL] ") << test.name << "\n";
    }
    std::cout << "\nТестов: " << registry().size() << ", проверок: " << checks << ", ошибок: " << failures << "\n";
    return failures == 0 ? 0 : 1;
}
