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
 * be.void.core.render.world.chunk — Simplex Noise implementation
 *
 * Ken Perlin's simplex noise, 2D.
 */

#include "Noise.h"
#include <algorithm>
#include <random>

namespace be::void_::core::render::world::chunk {

/* Градиенты для 2D simplex */
const std::array<int, 32> Noise::GRAD = {
    1,1,  -1,1,  1,-1,  -1,-1,
    1,0,  -1,0,  0,1,   0,-1,
    1,1,  -1,1,  1,-1,  -1,-1,
    1,0,  -1,0,  0,1,   0,-1,
};

/* Перемешивание для seed */
static uint32_t mix(uint32_t x) {
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    x *= 0x45d9f3bU;
    x ^= x >> 16;
    return x;
}

Noise::Noise(uint32_t seed) {
    std::array<int, 256> p;
    for (int i = 0; i < 256; i++) p[i] = i;

    /* Fisher-Yates shuffle с seed */
    std::mt19937 rng(mix(seed));
    for (int i = 255; i > 0; i--) {
        std::uniform_int_distribution<int> dist(0, i);
        int j = dist(rng);
        std::swap(p[i], p[j]);
    }

    for (int i = 0; i < 512; i++) m_perm[i] = p[i & 255];
}

static float dot2(const int* g, float x, float y) {
    return g[0]*x + g[1]*y;
}

float Noise::noise2D(float x, float y) const {
    int i = static_cast<int>(std::floor(x + y * F2));
    int j = static_cast<int>(std::floor(y + x * F2));

    float t = (i + j) * G2;
    float X0 = i - t, Y0 = j - t;
    float x0 = x - X0, y0 = y - Y0;

    int i1 = (x0 > y0) ? 1 : 0;
    int j1 = (x0 > y0) ? 0 : 1;

    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    int ii = i & 255;
    int jj = j & 255;

    float n0 = 0, n1 = 0, n2 = 0;

    float t0 = 0.5f - x0*x0 - y0*y0;
    if (t0 >= 0) {
        t0 *= t0;
        int gi0 = m_perm[ii + m_perm[jj]] & 15;
        n0 = t0 * t0 * dot2(&GRAD[gi0*2], x0, y0);
    }

    float t1 = 0.5f - x1*x1 - y1*y1;
    if (t1 >= 0) {
        t1 *= t1;
        int gi1 = m_perm[ii + i1 + m_perm[jj + j1]] & 15;
        n1 = t1 * t1 * dot2(&GRAD[gi1*2], x1, y1);
    }

    float t2 = 0.5f - x2*x2 - y2*y2;
    if (t2 >= 0) {
        t2 *= t2;
        int gi2 = m_perm[ii + 1 + m_perm[jj + 1]] & 15;
        n2 = t2 * t2 * dot2(&GRAD[gi2*2], x2, y2);
    }

    return 70.0f * (n0 + n1 + n2);
}

float Noise::octaveNoise2D(float x, float y, int octaves) const {
    float total = 0, frequency = 1, amplitude = 1, maxVal = 0;

    for (int i = 0; i < octaves; i++) {
        total += noise2D(x * frequency, y * frequency) * amplitude;
        maxVal += amplitude;
        amplitude *= PERSISTENCE;
        frequency *= LACUNARITY;
    }

    return total / maxVal; /* нормализация [-1, 1] */
}

} // namespace be::void_::core::render::world::chunk
