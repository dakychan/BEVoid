/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#include "BiomeNoise.h"
#include <algorithm>

namespace be::void_::core::render::world::biome {

BiomeNoise::BiomeNoise(unsigned int seed) : m_seed(seed)
{
    m_heightNoise = chunk::Noise(seed);
    m_tempNoise = chunk::Noise(seed + 1);
    m_humidityNoise = chunk::Noise(seed + 2);
}

BiomeSample BiomeNoise::sample(float wx, float wz) const
{
    BiomeSample result;
    
    // 1. ВЫСОТА - ТОЛЬКО от шума, НЕ зависит от биома вообще
    float heightNoise = m_heightNoise.octaveNoise2D(wx * 0.02f, wz * 0.02f, 4);
    result.height = heightNoise * 0.5f + 0.5f;  // 0..1
    
    // 2. ТЕМПЕРАТУРА - для определения снега/травы
    result.temperature = m_tempNoise.octaveNoise2D(wx * 0.0033f, wz * 0.0033f, 2) * 0.5f + 0.5f;
    
    // 3. ВЛАЖНОСТЬ - для определения биома
    result.humidity = m_humidityNoise.octaveNoise2D(wx * 0.0033f, wz * 0.0033f, 2) * 0.5f + 0.5f;
    
    // Определение биома по температуре и влажности (ТОЛЬКО для цвета)
    result.biomeType = Biome::selectBiome(result.temperature, result.humidity);
    
    return result;
}

} // namespace
