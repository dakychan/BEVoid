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
 * be.void.core.render.world.chunk — Simplex Noise
 *
 * Быстрый процедурный шум для генерации террейна.
 * Многооктавный — 4 октавы для реалистичного рельефа.
 */

#ifndef BE_VOID_CORE_RENDER_WORLD_CHUNK_NOISE_H
#define BE_VOID_CORE_RENDER_WORLD_CHUNK_NOISE_H

#include <cstdint>
#include <cmath>
#include <array>
#include <random>

namespace be::void_::core::render::world::chunk {

class Noise {
public:
    Noise(uint32_t seed = 12345);

    /* 2D Simplex noise, возвращает [-1, 1] */
    float noise2D(float x, float y) const;

    /* Фрактальный шум (4 октавы), [-1, 1] */
    float octaveNoise2D(float x, float y, int octaves = 4) const;

    /* Персистентность между октавами */
    static constexpr float PERSISTENCE = 0.5f;
    static constexpr float LACUNARITY  = 2.0f;

private:
    std::array<int, 512> m_perm;

    static constexpr float F2 = 0.366025403784f;  /* 0.5 * (sqrt(3) - 1) */
    static constexpr float G2 = 0.211324865405f;  /* (3 - sqrt(3)) / 6 */

    static const std::array<int, 16*2> GRAD;
};

} // namespace be::void_::core::render::world::chunk

#endif // BE_VOID_CORE_RENDER_WORLD_CHUNK_NOISE_H
