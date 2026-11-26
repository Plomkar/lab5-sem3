#pragma once

#include <vector>

#include "terrain.hpp"

namespace wg {

class TerrainFactory {
public:
    TerrainFactory() = delete;

    static const Terrain& create(double elevation, double moisture);
    static const std::vector<const Terrain*>& all();
    static const Terrain* bySymbol(char symbol);

    static constexpr double kSeaLevel = 0.37;
    static constexpr double kBeachLevel = 0.40;
    static constexpr double kMountainLevel = 0.69;
    static constexpr double kSnowLevel = 0.83;
    static constexpr double kDryLimit = 0.40;
    static constexpr double kWetLimit = 0.58;
};

}  // namespace wg
