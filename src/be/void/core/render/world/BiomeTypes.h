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
    Ocean   = 6,
    Shore   = 12,
    Depot      = 7,
    Road       = 8,
    House      = 9,
    BusStop    = 10,
    Wasteland  = 11
};

enum class BuildingQuality : uint8_t {
    New       = 0,
    Normal    = 1,
    Old       = 2,
    Ruined    = 3,
    Abandoned = 4
};

struct BiomeColor { float r, g, b; };

struct BiomeInfo {
    Biome      type;
    float      baseHeight;
    float      variation;
    BiomeColor color;
};

struct BuildingInfo {
    float      posX, posZ;
    float      sizeX, sizeZ;
    float      height;
    BuildingQuality quality;
    Biome      type;
};

struct WireSegment {
    float x1, z1, x2, z2;
    float height1, height2;
    float sag;
};

struct PoleInfo {
    float x, z;
    float height;
    bool hasWires;
};

inline BiomeColor biomeColor(Biome type, float height, float hum, float ridgeVal, float variation = 0.0f) {
    auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };
    
    float h = height;

    float dr, dg, db;
    float sr, sg, sb;
    float gr, gg, gb;
    float cr, cg, cb;
    float snr, sng, snb;

    switch (type) {
    case Biome::Depot:
        dr = 0.15f; dg = 0.15f; db = 0.15f;
        sr = 0.25f; sg = 0.22f; sb = 0.20f;
        gr = 0.30f; gg = 0.27f; gb = 0.25f;
        cr = 0.35f; cg = 0.32f; cb = 0.30f;
        snr = 0.4f; sng = 0.4f; snb = 0.4f;
        break;
    case Biome::Road:
        dr = 0.18f; dg = 0.18f; db = 0.18f;
        sr = 0.22f; sg = 0.22f; sb = 0.22f;
        gr = 0.25f; gg = 0.25f; gb = 0.25f;
        cr = 0.28f; cg = 0.26f; cb = 0.24f;
        snr = 0.3f; sng = 0.3f; snb = 0.3f;
        break;
    case Biome::House:
        dr = 0.20f; dg = 0.18f; db = 0.16f;
        sr = 0.35f; sg = 0.30f; sb = 0.25f;
        gr = 0.45f; gg = 0.40f; gb = 0.35f;
        cr = 0.50f; cg = 0.45f; cb = 0.40f;
        snr = 0.55f; sng = 0.50f; snb = 0.45f;
        break;
    case Biome::BusStop:
        dr = 0.25f; dg = 0.25f; db = 0.28f;
        sr = 0.40f; sg = 0.42f; sb = 0.45f;
        gr = 0.50f; gg = 0.52f; gb = 0.55f;
        cr = 0.60f; cg = 0.58f; cb = 0.55f;
        snr = 0.65f; sng = 0.65f; snb = 0.68f;
        break;
    case Biome::Wasteland:
        dr = 0.08f; dg = 0.10f; db = 0.08f;
        sr = 0.15f; sg = 0.18f; sb = 0.15f;
        gr = 0.25f; gg = 0.28f; gb = 0.25f;
        cr = 0.35f; cg = 0.32f; cb = 0.30f;
        snr = 0.4f; sng = 0.38f; snb = 0.35f;
        break;
    case Biome::Shore:
        dr = 0.05f; dg = 0.08f; db = 0.35f;
        sr = 0.60f; sg = 0.55f; sb = 0.35f;
        gr = 0.70f; gg = 0.65f; gb = 0.45f;
        cr = 0.50f; cg = 0.45f; cb = 0.40f;
        snr = 0.85f; sng = 0.82f; snb = 0.78f;
        break;
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

    float r, g, b;
    
    if (type == Biome::Road || type == Biome::Depot) {
        r = gr; g = gg; b = gb;
    } else if (h < 0.10f) {
        float t = h / 0.10f;
        r = lerp(dr, sr, t); g = lerp(dg, sg, t); b = lerp(db, sb, t);
    } else if (h < 0.20f) {
        float t = (h - 0.10f) / 0.10f;
        r = lerp(sr, gr, t); g = lerp(sg, gg, t); b = lerp(sb, gb, t);
    } else if (h < 0.55f) {
        float v = variation * 0.08f;
        r = gr + v; g = gg + v; b = gb + v * 0.5f;
    } else if (h < 0.70f) {
        float t = (h - 0.55f) / 0.15f;
        float tr = lerp(gr, cr, t);
        float tg = lerp(gg, cg, t);
        float tb = lerp(gb, cb, t);
        float v = variation * 0.05f;
        r = tr + v; g = tg + v; b = tb + v * 0.3f;
    } else if (h < 0.85f) {
        float t = (h - 0.70f) / 0.15f;
        r = lerp(cr, snr, t); g = lerp(cg, sng, t); b = lerp(cb, snb, t);
    } else {
        r = snr; g = sng; b = snb;
    }

    if (ridgeVal > 0.3f) {
        float rockMix = (ridgeVal - 0.3f) / 0.7f;
        rockMix = rockMix * rockMix;
        r = lerp(r, cr, rockMix);
        g = lerp(g, cg, rockMix);
        b = lerp(b, cb, rockMix);
    }

    return {r, g, b};
}

inline BiomeColor buildingColor(BuildingQuality q) {
    switch (q) {
    case BuildingQuality::New:
        return {0.55f, 0.52f, 0.48f};
    case BuildingQuality::Normal:
        return {0.45f, 0.42f, 0.38f};
    case BuildingQuality::Old:
        return {0.35f, 0.32f, 0.28f};
    case BuildingQuality::Ruined:
        return {0.25f, 0.22f, 0.20f};
    case BuildingQuality::Abandoned:
        return {0.20f, 0.18f, 0.15f};
    default:
        return {0.40f, 0.38f, 0.35f};
    }
}

} // namespace
#endif
