#include "map_generator.hpp"

#include <algorithm>
#include <stdexcept>

#include "terrain_factory.hpp"

namespace wg {

namespace {

void validate(const GeneratorConfig& config) {
    if (config.width < 1 || config.width > 300) {
        throw std::invalid_argument("Ширина карты должна быть от 1 до 300");
    }
    if (config.height < 1 || config.height > 200) {
        throw std::invalid_argument("Высота карты должна быть от 1 до 200");
    }
    if (config.scale < 1.0 || config.scale > 1000.0) {
        throw std::invalid_argument("Масштаб шума должен быть от 1 до 1000");
    }
    if (config.octaves < 1 || config.octaves > 8) {
        throw std::invalid_argument("Число октав должно быть от 1 до 8");
    }
    if (config.persistence <= 0.0 || config.persistence > 1.0) {
        throw std::invalid_argument("Затухание октав должно быть в диапазоне (0; 1]");
    }
    if (config.aspect <= 0.0 || config.aspect > 10.0) {
        throw std::invalid_argument("Соотношение сторон клетки должно быть в диапазоне (0; 10]");
    }
}

}  // namespace

MapGenerator::MapGenerator(const GeneratorConfig& config) : config_(config) {
    validate(config_);
}

std::vector<double> MapGenerator::makeField(const PerlinNoise& noise) const {
    std::vector<double> field;
    field.reserve(static_cast<std::size_t>(config_.width) * static_cast<std::size_t>(config_.height));
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            const double nx = x / config_.scale;
            const double ny = y * config_.aspect / config_.scale;
            field.push_back(noise.octaveNoise(nx, ny, config_.octaves, config_.persistence));
        }
    }

    const auto [lowest, highest] = std::minmax_element(field.begin(), field.end());
    const double low = *lowest;
    const double range = *highest - low;
    for (double& value : field) {
        value = range > 0.0 ? (value - low) / range : 0.5;
    }
    return field;
}

WorldMap MapGenerator::generate() const {
    const PerlinNoise elevationNoise(config_.seed);
    const PerlinNoise moistureNoise(config_.seed * 2654435761U + 1013904223U);

    const std::vector<double> elevation = makeField(elevationNoise);
    const std::vector<double> moisture = makeField(moistureNoise);

    WorldMap map(config_.width, config_.height, *TerrainFactory::all().front());
    std::size_t index = 0;
    for (int y = 0; y < config_.height; ++y) {
        for (int x = 0; x < config_.width; ++x) {
            map.set(x, y, TerrainFactory::create(elevation[index], moisture[index]));
            ++index;
        }
    }
    return map;
}

MapGeneratorBuilder& MapGeneratorBuilder::width(int value) {
    config_.width = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::height(int value) {
    config_.height = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::seed(std::uint32_t value) {
    config_.seed = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::scale(double value) {
    config_.scale = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::octaves(int value) {
    config_.octaves = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::persistence(double value) {
    config_.persistence = value;
    return *this;
}

MapGeneratorBuilder& MapGeneratorBuilder::aspect(double value) {
    config_.aspect = value;
    return *this;
}

MapGenerator MapGeneratorBuilder::build() const {
    return MapGenerator(config_);
}

}  // namespace wg
