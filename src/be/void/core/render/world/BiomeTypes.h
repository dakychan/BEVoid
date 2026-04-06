#ifndef BEVOID_WORLD_BIOME_TYPES_H
#define BEVOID_WORLD_BIOME_TYPES_H

#include <cstdint>

namespace be::void_::core::render::world {

enum class Biome : uint8_t {
    Tundra  = 0,
    Desert  = 1,
    Forest  = 2,
    Swamp   = 3,
    Savanna = 4,
    Jungle  = 5,
    Ocean   = 6
};

struct BiomeColor { float r, g, b; };

struct BiomeInfo {
    Biome      type;
    float      baseHeight;
    float      variation;
    BiomeColor color;
};

// Все биомы в таблице
inline BiomeInfo biomeInfo(Biome t) {
    static const BiomeInfo TABLE[] = {
        {Biome::Tundra,  0.25f, 0.15f, {0.90f, 0.95f, 1.00f}},
        {Biome::Desert,  0.20f, 0.10f, {0.95f, 0.85f, 0.60f}},
        {Biome::Forest,  0.30f, 0.15f, {0.20f, 0.60f, 0.20f}},
        {Biome::Swamp,   0.15f, 0.10f, {0.30f, 0.50f, 0.30f}},
        {Biome::Savanna, 0.25f, 0.15f, {0.80f, 0.70f, 0.40f}},
        {Biome::Jungle,  0.30f, 0.20f, {0.10f, 0.50f, 0.10f}},
        {Biome::Ocean,   0.10f, 0.05f, {0.10f, 0.30f, 0.80f}},
    };
    return TABLE[(int)t];
}

} // namespace
#endif
