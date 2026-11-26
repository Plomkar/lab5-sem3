#include "terrain_factory.hpp"

namespace wg {

namespace {

const Sea kSea;
const Beach kBeach;
const Desert kDesert;
const Plains kPlains;
const Forest kForest;
const Mountain kMountain;
const Snow kSnow;

}  // namespace

const Terrain& TerrainFactory::create(double elevation, double moisture) {
    if (elevation < kSeaLevel) {
        return kSea;
    }
    if (elevation < kBeachLevel) {
        return kBeach;
    }
    if (elevation >= kSnowLevel) {
        return kSnow;
    }
    if (elevation >= kMountainLevel) {
        return kMountain;
    }
    if (moisture < kDryLimit) {
        return kDesert;
    }
    if (moisture < kWetLimit) {
        return kPlains;
    }
    return kForest;
}

const std::vector<const Terrain*>& TerrainFactory::all() {
    static const std::vector<const Terrain*> terrains = {&kSea,    &kBeach,    &kDesert, &kPlains,
                                                         &kForest, &kMountain, &kSnow};
    return terrains;
}

const Terrain* TerrainFactory::bySymbol(char symbol) {
    for (const Terrain* terrain : all()) {
        if (terrain->symbol() == symbol) {
            return terrain;
        }
    }
    return nullptr;
}

}  // namespace wg
