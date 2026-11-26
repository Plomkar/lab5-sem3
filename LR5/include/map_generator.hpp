#pragma once

#include <cstdint>
#include <vector>

#include "perlin_noise.hpp"
#include "world_map.hpp"

namespace wg {

struct GeneratorConfig {
    int width = 60;
    int height = 20;
    std::uint32_t seed = 1;
    double scale = 18.0;
    int octaves = 4;
    double persistence = 0.5;
    double aspect = 2.0;
};

class MapGenerator {
public:
    explicit MapGenerator(const GeneratorConfig& config);

    const GeneratorConfig& config() const noexcept { return config_; }
    WorldMap generate() const;

private:
    std::vector<double> makeField(const PerlinNoise& noise) const;

    GeneratorConfig config_;
};

class MapGeneratorBuilder {
public:
    MapGeneratorBuilder& width(int value);
    MapGeneratorBuilder& height(int value);
    MapGeneratorBuilder& seed(std::uint32_t value);
    MapGeneratorBuilder& scale(double value);
    MapGeneratorBuilder& octaves(int value);
    MapGeneratorBuilder& persistence(double value);
    MapGeneratorBuilder& aspect(double value);

    MapGenerator build() const;

private:
    GeneratorConfig config_;
};

}  // namespace wg
