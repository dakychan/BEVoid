/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#include "Biome.h"

namespace be::void_::core::render::world::biome {

BiomeParams Biome::getParams(BiomeType type)
{
    BiomeParams params;
    params.type = type;
    
    switch (type) {
        case BiomeType::TUNDRA:
            params.baseHeight = 0.25f;
            params.heightVariation = 0.15f;
            params.maxSize = 300.0f;
            params.color = {0.9f, 0.95f, 1.0f};  // Белый/голубой
            break;
            
        case BiomeType::DESERT:
            params.baseHeight = 0.2f;
            params.heightVariation = 0.1f;
            params.maxSize = 300.0f;
            params.color = {0.95f, 0.85f, 0.6f};  // Песочный
            break;
            
        case BiomeType::FOREST:
            params.baseHeight = 0.3f;
            params.heightVariation = 0.15f;
            params.maxSize = 300.0f;
            params.color = {0.2f, 0.6f, 0.2f};  // Зеленый
            break;
            
        case BiomeType::SWAMP:
            params.baseHeight = 0.15f;
            params.heightVariation = 0.1f;
            params.maxSize = 300.0f;
            params.color = {0.3f, 0.5f, 0.3f};  // Темно-зеленый
            break;
            
        case BiomeType::SAVANNA:
            params.baseHeight = 0.25f;
            params.heightVariation = 0.15f;
            params.maxSize = 300.0f;
            params.color = {0.8f, 0.7f, 0.4f};  // Желто-коричневый
            break;
            
        case BiomeType::JUNGLE:
            params.baseHeight = 0.3f;
            params.heightVariation = 0.2f;
            params.maxSize = 300.0f;
            params.color = {0.1f, 0.5f, 0.1f};  // Темно-зеленый
            break;
            
        case BiomeType::OCEAN:
            params.baseHeight = 0.1f;  // Минимум 20м, не 0!
            params.heightVariation = 0.05f;
            params.maxSize = 300.0f;
            params.color = {0.1f, 0.3f, 0.8f};  // Синий
            break;
    }
    
    return params;
}

BiomeType Biome::selectBiome(float temperature, float humidity)
{
    // Температура и влажность от 0.0 до 1.0
    if (temperature < 0.3f) {
        return BiomeType::TUNDRA;
    } else if (temperature < 0.6f) {
        if (humidity < 0.3f) return BiomeType::DESERT;
        else if (humidity < 0.7f) return BiomeType::FOREST;
        else return BiomeType::SWAMP;
    } else {
        if (humidity < 0.3f) return BiomeType::SAVANNA;
        else if (humidity < 0.7f) return BiomeType::JUNGLE;
        else return BiomeType::OCEAN;
    }
}

BiomeColor Biome::getColor(BiomeType type)
{
    return getParams(type).color;
}

} // namespace
