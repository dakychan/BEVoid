#ifndef BEVOID_WORLD_CONSTANTS_H
#define BEVOID_WORLD_CONSTANTS_H

namespace be::void_::core::render::world {

/* ---- Chunk ---- */
constexpr int    CHUNK_SIZE   = 32;
constexpr int    CHUNK_GRID   = 33;
constexpr int    RENDER_DIST  = 8;

/* ---- Terrain - FLAT WORLD ---- */
constexpr float  MAX_HEIGHT   = 5.0f;
constexpr float  GROUND_LEVEL = 0.0f;
constexpr float  WATER_LEVEL  = -1.0f;
constexpr float  SNOW_LINE    = 100.0f;

/* ---- Trolleybus System ---- */
constexpr float  WIRE_HEIGHT_MIN    = 5.2f;
constexpr float  WIRE_HEIGHT_MAX    = 5.8f;
constexpr float  WIRE_HEIGHT_NORMAL = 5.75f;
constexpr float  POLE_HEIGHT        = 7.5f;
constexpr float  POLE_SPACING       = 40.0f;

/* ---- Depot ---- */
constexpr float  DEPOT_SIZE_X       = 60.0f;
constexpr float  DEPOT_SIZE_Z       = 80.0f;
constexpr float  DEPOT_WALL_HEIGHT  = 8.0f;
constexpr float  DEPOT_SPAWN_X      = 0.0f;
constexpr float  DEPOT_SPAWN_Z      = 0.0f;

/* ---- Structures ---- */
constexpr float  HOUSE_MIN_SIZE     = 8.0f;
constexpr float  HOUSE_MAX_SIZE     = 20.0f;
constexpr float  HOUSE_MIN_HEIGHT   = 6.0f;
constexpr float  HOUSE_MAX_HEIGHT   = 15.0f;
constexpr float  HOUSE_DENSITY      = 0.15f;
constexpr float  BUS_STOP_DENSITY   = 0.03f;
constexpr float  ROAD_WIDTH         = 12.0f;

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
