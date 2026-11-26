#pragma once

#include <cstddef>
#include <vector>

#include "terrain.hpp"

namespace wg {

struct TerrainStat {
    const Terrain* terrain;
    int count;
    double percent;
};

class WorldMap {
public:
    WorldMap(int width, int height, const Terrain& fill);

    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

    const Terrain& at(int x, int y) const;
    void set(int x, int y, const Terrain& terrain);

    std::vector<TerrainStat> statistics() const;
    double walkablePercent() const;

private:
    std::size_t index(int x, int y) const;

    int width_;
    int height_;
    std::vector<const Terrain*> cells_;
};

}  // namespace wg
