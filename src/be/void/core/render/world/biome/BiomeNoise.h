/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

/*
 * be.void.core.render.world.biome — BiomeNoise
 *
 * Генерация биомов с ограниченными размерами.
 * Использует температуру и влажность для определения биома.
 */

#ifndef BE_VOID_CORE_RENDER_WORLD_BIOME_NOISE_H
#define BE_VOID_CORE_RENDER_WORLD_BIOME_NOISE_H

#include "Biome.h"
#include "../chunk/Noise.h"
#include <cmath>
#include <algorithm>

namespace be::void_::core::render::world::biome {

struct BiomeSample {
    float height;
    float temperature;
    float humidity;
    BiomeType biomeType;
};

class BiomeNoise {
public:
    explicit BiomeNoise(uint32_t seed);
    
    BiomeSample sample(float wx, float wz) const;
    
private:
    mutable chunk::Noise m_heightNoise;
    mutable chunk::Noise m_tempNoise;
    mutable chunk::Noise m_humidityNoise;
    unsigned int m_seed;
};

inline float smoothstep(float edge0, float edge1, float x) {
    float t = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0f - 2.0f * t);
}

} // namespace be::void_::core::render::world::biome

#endif // BEVOID_BIOME_NOISE_H
