#include "world_map.hpp"

#include <stdexcept>
#include <string>

#include "terrain_factory.hpp"

namespace wg {

namespace {

std::size_t cellCount(int width, int height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Размеры карты должны быть положительными");
    }
    return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
}

}  // namespace

WorldMap::WorldMap(int width, int height, const Terrain& fill)
    : width_(width), height_(height), cells_(cellCount(width, height), &fill) {}

std::size_t WorldMap::index(int x, int y) const {
    if (x < 0 || y < 0 || x >= width_ || y >= height_) {
        throw std::out_of_range("Клетка (" + std::to_string(x) + ", " + std::to_string(y) + ") вне карты");
    }
    return static_cast<std::size_t>(y) * static_cast<std::size_t>(width_) + static_cast<std::size_t>(x);
}

const Terrain& WorldMap::at(int x, int y) const {
    return *cells_[index(x, y)];
}

void WorldMap::set(int x, int y, const Terrain& terrain) {
    cells_[index(x, y)] = &terrain;
}

std::vector<TerrainStat> WorldMap::statistics() const {
    std::vector<TerrainStat> stats;
    const double total = static_cast<double>(cells_.size());
    for (const Terrain* terrain : TerrainFactory::all()) {
        int count = 0;
        for (const Terrain* cell : cells_) {
            if (cell == terrain) {
                ++count;
            }
        }
        stats.push_back({terrain, count, 100.0 * count / total});
    }
    return stats;
}

double WorldMap::walkablePercent() const {
    int walkable = 0;
    for (const Terrain* cell : cells_) {
        if (cell->isWalkable()) {
            ++walkable;
        }
    }
    return 100.0 * walkable / static_cast<double>(cells_.size());
}

}  // namespace wg
