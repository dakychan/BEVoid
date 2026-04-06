#ifndef BEVOID_WORLD_NOISE_H
#define BEVOID_WORLD_NOISE_H

#include <cstdint>
#include <array>

namespace be::void_::core::render::world {

class Noise {
public:
    explicit Noise(uint32_t seed = 0);

    // Simplex 2D, возврат [-1, 1]
    float sample(float x, float y) const;

    // Фрактальный шум, [-1, 1]
    float octave(float x, float y, int octaves) const;

    // Ridged noise (|n|), [0, 1] — для рек/обрывов
    float ridge(float x, float y, int octaves) const;

private:
    std::array<int, 512> perm{};

    static constexpr float F2 = 0.366025403784f;
    static constexpr float G2 = 0.211324865405f;
    static inline const int GRAD[32] = {
        1,1, -1,1, 1,-1, -1,-1,
        1,0, -1,0, 0,1,  0,-1,
        1,1, -1,1, 1,-1, -1,-1,
        1,0, -1,0, 0,1,  0,-1
    };
};

} // namespace
#endif
