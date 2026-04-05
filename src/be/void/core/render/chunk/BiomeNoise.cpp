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
 * be.void.core.render.chunk — BiomeNoise implementation
 *
 * Континентальный шум: scale=0.008, 2 октавы → большие области
 * Детальный шум: scale=0.03, 4 октавы → мелкий рельеф
 * Горный шум: scale=0.005, 3 октавы → где будут горы
 */

#include "core/render/chunk/BiomeNoise.h"

namespace be::void_::core::render::chunk {

BiomeNoise::BiomeNoise(uint32_t seed)
    : m_continent(seed), m_detail(seed + 1000), m_mountain(seed + 2000) {}

TerrainSample BiomeNoise::sample(float worldX, float worldZ) const {
    /* Континентальный шум — определяет биом */
    float continent = m_continent.octaveNoise2D(worldX * 0.008f, worldZ * 0.008f, 2);
    continent = continent * 0.5f + 0.5f; /* [-1,1] → [0,1] */

    /* Горный шум — добавляет горы в определённых зонах */
    float mountain = m_mountain.octaveNoise2D(worldX * 0.005f, worldZ * 0.005f, 3);
    mountain = mountain * 0.5f + 0.5f;

    /* Детальный шум — рельеф */
    float detail = m_detail.octaveNoise2D(worldX * 0.03f, worldZ * 0.03f, 4);
    detail = detail * 0.5f + 0.5f;

    /* Комбинируем */
    float baseHeight = continent * 0.4f + mountain * 0.6f;
    float height;

    /* Биомы и их высота */
    if (baseHeight < 0.25f) {
        /* Океан — плоский, чуть шумит */
        height = baseHeight * 0.3f + detail * 0.05f;
    } else if (baseHeight < 0.35f) {
        /* Пляж — плавный переход */
        float t = smoothstep(0.25f, 0.35f, baseHeight);
        float oceanH = 0.25f * 0.3f + detail * 0.05f;
        float plainsH = 0.35f + detail * 0.15f;
        height = oceanH * (1.0f - t) + plainsH * t;
    } else if (baseHeight < 0.55f) {
        /* Равнина */
        height = 0.35f + detail * 0.15f + mountain * 0.05f;
    } else if (baseHeight < 0.75f) {
        /* Холмы */
        float t = smoothstep(0.55f, 0.75f, baseHeight);
        float plainsH = 0.35f + detail * 0.15f;
        float hillsH = 0.5f + detail * 0.25f + mountain * 0.1f;
        height = plainsH * (1.0f - t) + hillsH * t;
    } else {
        /* Горы */
        float t = smoothstep(0.75f, 1.0f, baseHeight);
        float hillsH = 0.5f + detail * 0.25f + mountain * 0.1f;
        float mountainH = 0.7f + detail * 0.3f + mountain * 0.15f;
        height = hillsH * (1.0f - t) + mountainH * t;
    }

    height = std::max(0.0f, std::min(1.0f, height));

    /* Определяем биом */
    Biome biome;
    if (height < WATER_LEVEL - 0.02f)      biome = Biome::Ocean;
    else if (height < WATER_LEVEL + 0.02f) biome = Biome::Beach;
    else if (height < 0.45f)                biome = Biome::Plains;
    else if (height < 0.6f)                 biome = Biome::Hills;
    else if (height < 0.8f)                 biome = Biome::Mountains;
    else                                    biome = Biome::Peaks;

    return { height, biome };
}

float BiomeNoise::getHeight(float worldX, float worldZ) const {
    return sample(worldX, worldZ).height;
}

} // namespace be::void_::core::render::chunk
