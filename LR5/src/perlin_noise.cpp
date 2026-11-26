#include "perlin_noise.hpp"

#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <utility>

namespace wg {

PerlinNoise::PerlinNoise(std::uint32_t seed) {
    std::array<int, 256> base{};
    std::iota(base.begin(), base.end(), 0);

    std::mt19937 engine(seed);
    for (std::size_t i = base.size() - 1; i > 0; --i) {
        const std::size_t j = engine() % (i + 1);
        std::swap(base[i], base[j]);
    }
    for (std::size_t i = 0; i < permutation_.size(); ++i) {
        permutation_[i] = base[i % base.size()];
    }
}

double PerlinNoise::fade(double t) {
    return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}

double PerlinNoise::lerp(double from, double to, double t) {
    return from + t * (to - from);
}

double PerlinNoise::gradient(int hash, double x, double y) {
    switch (hash & 3) {
        case 0:
            return x + y;
        case 1:
            return -x + y;
        case 2:
            return x - y;
        default:
            return -x - y;
    }
}

double PerlinNoise::noise(double x, double y) const {
    const double floorX = std::floor(x);
    const double floorY = std::floor(y);
    const auto cellX = static_cast<std::size_t>(static_cast<long long>(floorX) & 255);
    const auto cellY = static_cast<std::size_t>(static_cast<long long>(floorY) & 255);

    const double localX = x - floorX;
    const double localY = y - floorY;
    const double u = fade(localX);
    const double v = fade(localY);

    const std::size_t a = static_cast<std::size_t>(permutation_[cellX]) + cellY;
    const std::size_t b = static_cast<std::size_t>(permutation_[cellX + 1]) + cellY;

    const double bottom = lerp(gradient(permutation_[a], localX, localY),
                               gradient(permutation_[b], localX - 1.0, localY), u);
    const double top = lerp(gradient(permutation_[a + 1], localX, localY - 1.0),
                            gradient(permutation_[b + 1], localX - 1.0, localY - 1.0), u);
    return lerp(bottom, top, v);
}

double PerlinNoise::octaveNoise(double x, double y, int octaves, double persistence) const {
    double total = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double amplitudeSum = 0.0;
    for (int octave = 0; octave < octaves; ++octave) {
        total += amplitude * noise(x * frequency, y * frequency);
        amplitudeSum += amplitude;
        amplitude *= persistence;
        frequency *= 2.0;
    }
    return total / amplitudeSum;
}

}  // namespace wg
