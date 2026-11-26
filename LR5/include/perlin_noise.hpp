#pragma once

#include <array>
#include <cstdint>

namespace wg {

class PerlinNoise {
public:
    explicit PerlinNoise(std::uint32_t seed);

    double noise(double x, double y) const;
    double octaveNoise(double x, double y, int octaves, double persistence) const;

private:
    static double fade(double t);
    static double lerp(double from, double to, double t);
    static double gradient(int hash, double x, double y);

    std::array<int, 512> permutation_{};
};

}  // namespace wg
