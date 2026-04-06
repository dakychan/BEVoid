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

// Процедурный цвет: биома + высота + шум вариации + обрывы
// heightNorm — нормализованная высота 0..1 от sea level
// variation — шум вариации цвета [-1, 1]
inline BiomeColor biomeColor(Biome type, float height, float hum, float ridgeVal, float variation = 0.0f) {
    auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

    // --- Палитра по высоте ---
    // height 0.00-0.10  →  глубокая вода (тёмно-синий)
    // height 0.10-0.20  →  мелководье → песок
    // height 0.20-0.30  →  песок → земля
    // height 0.30-0.60  →  трава (зависит от биома)
    // height 0.60-0.75  →  камень
    // height 0.75+      →  снег

    float h = height;  // 0..1

    // Базовые цвета слоёв
    float dr, dg, db;  // deep water
    float sr, sg, sb;  // shallow/sand
    float gr, gg, gb;  // ground/grass
    float cr, cg, cb;  // cliff/rock
    float snr, sng, snb;  // snow

    switch (type) {
    case Biome::Ocean:
        dr = 0.03f; dg = 0.06f; db = 0.35f;
        sr = 0.06f; sg = 0.15f; sb = 0.50f;
        gr = 0.10f; gg = 0.20f; gb = 0.45f;
        cr = 0.15f; cg = 0.20f; cb = 0.40f;
        snr = 0.6f; sng = 0.7f; snb = 0.85f;
        break;
    case Biome::Desert:
        dr = 0.03f; dg = 0.06f; db = 0.30f;
        sr = 0.80f; sg = 0.68f; sb = 0.40f;
        gr = 0.85f; gg = 0.73f; gb = 0.45f;
        cr = 0.70f; cg = 0.60f; cb = 0.45f;
        snr = 0.9f; sng = 0.88f; snb = 0.85f;
        break;
    case Biome::Forest: {
        // Зелень зависит от влажности
        float green = lerp(0.35f, 0.55f, hum);
        float red   = lerp(0.10f, 0.25f, hum);
        float blue  = lerp(0.05f, 0.12f, hum);
        dr = 0.03f; dg = 0.06f; db = 0.30f;
        sr = 0.50f; sg = 0.42f; sb = 0.25f;
        gr = red;   gg = green; gb = blue;
        cr = 0.45f; cg = 0.42f; cb = 0.38f;
        snr = 0.9f; sng = 0.92f; snb = 0.95f;
        break;
    }
    case Biome::Jungle:
        dr = 0.02f; dg = 0.05f; db = 0.25f;
        sr = 0.55f; sg = 0.45f; sb = 0.20f;
        gr = 0.06f; gg = 0.45f; gb = 0.06f;
        cr = 0.40f; cg = 0.35f; cb = 0.30f;
        snr = 0.85f; sng = 0.88f; snb = 0.92f;
        break;
    case Biome::Swamp:
        dr = 0.03f; dg = 0.08f; db = 0.25f;
        sr = 0.35f; sg = 0.32f; sb = 0.18f;
        gr = 0.22f; gg = 0.38f; gb = 0.18f;
        cr = 0.35f; cg = 0.30f; cb = 0.25f;
        snr = 0.85f; sng = 0.88f; snb = 0.92f;
        break;
    case Biome::Savanna:
        dr = 0.03f; dg = 0.06f; db = 0.30f;
        sr = 0.70f; sg = 0.58f; sb = 0.32f;
        gr = 0.72f; gg = 0.62f; gb = 0.30f;
        cr = 0.55f; cg = 0.48f; cb = 0.38f;
        snr = 0.9f; sng = 0.88f; snb = 0.85f;
        break;
    case Biome::Tundra:
        dr = 0.04f; dg = 0.08f; db = 0.28f;
        sr = 0.55f; sg = 0.58f; sb = 0.60f;
        gr = 0.60f; gg = 0.62f; gb = 0.65f;
        cr = 0.45f; cg = 0.42f; cb = 0.40f;
        snr = 0.95f; sng = 0.95f; snb = 0.98f;
        break;
    default:
        dr = 0.05f; dg = 0.08f; db = 0.30f;
        sr = 0.50f; sg = 0.45f; sb = 0.30f;
        gr = 0.30f; gg = 0.50f; gb = 0.20f;
        cr = 0.50f; cg = 0.45f; cb = 0.40f;
        snr = 0.95f; sng = 0.95f; snb = 0.98f;
    }

    // --- Интерполяция по высоте ---
    float r, g, b;
    if (h < 0.10f) {
        float t = h / 0.10f;
        r = lerp(dr, sr, t); g = lerp(dg, sg, t); b = lerp(db, sb, t);
    } else if (h < 0.20f) {
        float t = (h - 0.10f) / 0.10f;
        r = lerp(sr, gr, t); g = lerp(sg, gg, t); b = lerp(sb, gb, t);
    } else if (h < 0.55f) {
        // Основная зона биома — трава/песок
        // Добавляем вариацию
        float v = variation * 0.08f;  // ±8% вариация
        r = gr + v; g = gg + v; b = gb + v * 0.5f;
    } else if (h < 0.70f) {
        // Переход трава → камень
        float t = (h - 0.55f) / 0.15f;
        float tr = lerp(gr, cr, t);
        float tg = lerp(gg, cg, t);
        float tb = lerp(gb, cb, t);
        float v = variation * 0.05f;
        r = tr + v; g = tg + v; b = tb + v * 0.3f;
    } else if (h < 0.85f) {
        // Камень → снег
        float t = (h - 0.70f) / 0.15f;
        r = lerp(cr, snr, t); g = lerp(cg, sng, t); b = lerp(cb, snb, t);
    } else {
        // Снег
        r = snr; g = sng; b = snb;
    }

    // Ridge = обрывы → камень, независимо от биома
    if (ridgeVal > 0.3f) {
        float rockMix = (ridgeVal - 0.3f) / 0.7f;
        rockMix = rockMix * rockMix;  // sharpen
        r = lerp(r, cr, rockMix);
        g = lerp(g, cg, rockMix);
        b = lerp(b, cb, rockMix);
    }

    return {r, g, b};
}

} // namespace
#endif
