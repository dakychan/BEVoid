#ifndef BEVOID_WORLD_CONSTANTS_H
#define BEVOID_WORLD_CONSTANTS_H

namespace be::void_::core::render::world {

/* ---- Chunk ---- */
constexpr int    CHUNK_SIZE   = 16;  // блоков
constexpr int    CHUNK_GRID   = 17;  // вершин (CHUNK_SIZE + 1)
constexpr int    RENDER_DIST  = 4;   // чанков

/* ---- Terrain ---- */
constexpr float  MAX_HEIGHT   = 60.0f;
constexpr float  WATER_LEVEL  = 10.0f;
constexpr float  SNOW_LINE    = 45.0f;

/* ---- Biome ---- */
constexpr float  BIOME_SIZE   = 300.0f;
constexpr float  BLEND_RADIUS = 10.0f;

/* ---- Noise ---- */
constexpr float  HEIGHT_FREQ  = 0.004f;
constexpr float  BIOME_FREQ   = 0.0033f;
constexpr int    OCTAVES      = 2;
constexpr float  PERSISTENCE  = 0.25f;
constexpr float  LACUNARITY   = 2.0f;

} // namespace
#endif
