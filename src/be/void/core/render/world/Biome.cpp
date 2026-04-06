#include "Biome.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include <algorithm>

namespace be::void_::core::render::world {

static Biome select(float temp, float hum) {
    if (temp < 0.3f) return Biome::Tundra;
    if (temp < 0.6f) {
        if (hum < 0.3f) return Biome::Desert;
        if (hum < 0.7f) return Biome::Forest;
        return Biome::Swamp;
    }
    if (hum < 0.3f) return Biome::Savanna;
    if (hum < 0.7f) return Biome::Jungle;
    return Biome::Ocean;
}

/* ---------- BiomeNoise ---------- */
BiomeNoise::BiomeNoise(uint32_t s)
    : heightNoise(s), tempNoise(s+1), humidityNoise(s+2) {}

BiomeSample BiomeNoise::sample(float wx, float wz) const {
    BiomeSample r;
    r.wx         = wx;
    r.wz         = wz;
    r.height     = heightNoise.octave(wx * HEIGHT_FREQ, wz * HEIGHT_FREQ, OCTAVES) * 0.5f + 0.5f;
    r.temperature = tempNoise.octave(wx * BIOME_FREQ, wz * BIOME_FREQ, 2) * 0.5f + 0.5f;
    r.humidity    = humidityNoise.octave(wx * BIOME_FREQ, wz * BIOME_FREQ, 2) * 0.5f + 0.5f;
    r.type        = select(r.temperature, r.humidity);
    return r;
}

} // namespace
