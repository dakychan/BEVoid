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
 * be.void.core.render.chunk — BiomeNoise
 *
 * Улучшенный шум для террейна:
 * - Континентальный шум (низкая частота) — биомы: океан/равнина/горы
 * - Детальный шум (высокая частота) — рельеф внутри биома
 * - Smooth интерполяция — нет "ступенек"
 */

#ifndef BEVOID_BIOME_NOISE_H
#define BEVOID_BIOME_NOISE_H

#include "core/render/chunk/Noise.h"
#include <cmath>
#include <algorithm>

namespace be::void_::core::render::chunk {

enum class Biome {
    Ocean,     /* вода */
    Beach,     /* пляж */
    Plains,    /* равнина */
    Hills,     /* холмы */
    Mountains, /* горы */
    Peaks      /* пики */
};

struct TerrainSample {
    float height;    /* 0..1 */
    Biome biome;
};

class BiomeNoise {
public:
    BiomeNoise(uint32_t seed = 12345);

    /* Получить высоту и биом в точке мира */
    TerrainSample sample(float worldX, float worldZ) const;

    /* Только высота (для быстрой коллизии) */
    float getHeight(float worldX, float worldZ) const;

    /* Уровень воды */
    static constexpr float WATER_LEVEL = 0.35f;

private:
    Noise m_continent;  /* биомы, низкая частота */
    Noise m_detail;     /* рельеф, высокая частота */
    Noise m_mountain;   /* где будут горы */
};

inline float smoothstep(float edge0, float edge1, float x) {
    float t = std::max(0.0f, std::min(1.0f, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0f - 2.0f * t);
}

} // namespace be::void_::core::render::chunk

#endif // BEVOID_BIOME_NOISE_H
