#include "renderers.hpp"

#include <cstddef>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <utility>

#include "terrain_factory.hpp"

namespace wg {

namespace {

const char* const kReset = "\033[0m";

std::string colorStart(const Terrain& terrain) {
    return "\033[" + std::to_string(terrain.colorCode()) + "m";
}

std::size_t displayWidth(const std::string& text) {
    std::size_t width = 0;
    for (const char symbol : text) {
        if ((static_cast<unsigned char>(symbol) & 0xC0U) != 0x80U) {
            ++width;
        }
    }
    return width;
}

std::string padRight(const std::string& text, std::size_t width) {
    const std::size_t current = displayWidth(text);
    return current >= width ? text : text + std::string(width - current, ' ');
}

std::string formatNumber(double value, int precision) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(precision) << value;
    return stream.str();
}

}  // namespace

std::string AsciiRenderer::render(const WorldMap& map) const {
    std::string text;
    text.reserve(static_cast<std::size_t>(map.width() + 1) * static_cast<std::size_t>(map.height()));
    for (int y = 0; y < map.height(); ++y) {
        for (int x = 0; x < map.width(); ++x) {
            text += map.at(x, y).symbol();
        }
        text += '\n';
    }
    return text;
}

RendererDecorator::RendererDecorator(std::unique_ptr<IMapRenderer> inner) : inner_(std::move(inner)) {
    if (!inner_) {
        throw std::invalid_argument("Декоратор должен оборачивать существующий отрисовщик");
    }
}

std::string FrameDecorator::render(const WorldMap& map) const {
    const std::string border = "+" + std::string(static_cast<std::size_t>(map.width()), '-') + "+\n";
    std::istringstream lines(inner().render(map));
    std::string result = border;
    std::string line;
    while (std::getline(lines, line)) {
        result += "|" + line + "|\n";
    }
    return result + border;
}

std::string ColorDecorator::render(const WorldMap& map) const {
    const std::string text = inner().render(map);
    std::string result;
    result.reserve(text.size() * 2);
    const Terrain* current = nullptr;
    for (const char symbol : text) {
        const Terrain* terrain = TerrainFactory::bySymbol(symbol);
        if (terrain != current) {
            result += terrain != nullptr ? colorStart(*terrain) : std::string(kReset);
            current = terrain;
        }
        result += symbol;
    }
    if (current != nullptr) {
        result += kReset;
    }
    return result;
}

LegendDecorator::LegendDecorator(std::unique_ptr<IMapRenderer> inner, bool colored)
    : RendererDecorator(std::move(inner)), colored_(colored) {}

std::string LegendDecorator::render(const WorldMap& map) const {
    std::string result = inner().render(map);
    result += "\nЛегенда:\n";
    for (const TerrainStat& stat : map.statistics()) {
        const std::string symbol(1, stat.terrain->symbol());
        const std::string shownSymbol = colored_ ? colorStart(*stat.terrain) + symbol + kReset : symbol;
        result += "  " + shownSymbol + "  " + padRight(stat.terrain->name(), 17);
        std::ostringstream count;
        count << std::setw(6) << stat.count << "  " << std::setw(5) << formatNumber(stat.percent, 1) << " %";
        result += count.str() + "\n";
    }
    result += "Проходимая территория: " + formatNumber(map.walkablePercent(), 1) + " %\n";
    return result;
}

std::string stripAnsi(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (std::size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\033' && i + 1 < text.size() && text[i + 1] == '[') {
            i += 2;
            while (i < text.size() && text[i] != 'm') {
                ++i;
            }
        } else {
            result += text[i];
        }
    }
    return result;
}

}  // namespace wg
