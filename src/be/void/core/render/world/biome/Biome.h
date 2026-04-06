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
 * be.void.core.render.world.biome — Biome system
 *
 * Биомы с ограниченными размерами (не миллионы метров).
 * Каждый биом имеет свои параметры высоты, цвета, блоков.
 */

#ifndef BE_VOID_CORE_RENDER_WORLD_BIOME_H
#define BE_VOID_CORE_RENDER_WORLD_BIOME_H

#include <cstdint>

namespace be::void_::core::render::world::biome {

enum class BiomeType : uint8_t {
    TUNDRA = 0,      // Снег/Тундра (холодно)
    DESERT = 1,      // Пустыня (жарко, сухо)
    FOREST = 2,      // Лес (умеренно)
    SWAMP = 3,       // Болото (влажно)
    SAVANNA = 4,     // Саванна (жарко, средняя влажность)
    JUNGLE = 5,      // Джунгли (жарко, влажно)
    OCEAN = 6        // Океан (вода)
};

struct BiomeColor {
    float r, g, b;
};

struct BiomeParams {
    BiomeType type;
    float baseHeight;        // Базовая высота (0.0 - 1.0)
    float heightVariation;   // Вариация высоты
    float maxSize;           // Максимальный размер биома в метрах
    BiomeColor color;
};

class Biome {
public:
    static BiomeParams getParams(BiomeType type);
    static BiomeType selectBiome(float temperature, float humidity);
    static BiomeColor getColor(BiomeType type);
    
private:
    static constexpr float BIOME_SIZE = 300.0f;  // Биом ~300 блоков (300 метров)
};

} // namespace

#endif // BE_VOID_CORE_RENDER_WORLD_BIOME_H
