#include "Biome.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include <algorithm>

namespace be::void_::core::render::world {

static float lerp(float a, float b, float t) { return a + (b - a) * t; }
static float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

/* ---------- Выбор биома по климату + высоте ---------- */
static Biome selectBiome(float temp, float hum, float height, float continental) {
    // Continental < 0.35 → океан, независимо от всего
    if (continental < 0.32f) return Biome::Ocean;

    // Берег → Ocean/Shallow
    if (continental < 0.40f) return Biome::Ocean;

    // Равнины
    if (height < 0.25f) {
        if (temp < 0.3f)  return Biome::Tundra;
        if (hum < 0.3f)   return Biome::Desert;
        if (hum < 0.6f)   return Biome::Forest;
        return Biome::Swamp;
    }

    // Холмы
    if (height < 0.55f) {
        if (hum < 0.3f)   return Biome::Savanna;
        if (hum < 0.6f)   return Biome::Forest;
        return Biome::Jungle;
    }

    // Горы — Forest/Tundra
    if (height > 0.85f) return Biome::Tundra;
    return Biome::Forest;
}

/* ---------- Процедурный цвет — теперь в BiomeTypes.h как biomeColor() ---------- */

/* ---------- BiomeNoise ---------- */
BiomeNoise::BiomeNoise(uint32_t s)
    : continental(s)
    , heightLo(s + 100)
    , ridge(s + 300)
    , temperature(s + 1000)
    , humidity(s + 2000)
{}

BiomeSample BiomeNoise::sample(float wx, float wz) const {
    BiomeSample r;
    r.wx = wx; r.wz = wz;

    // 1. Континент — ОГРОМНЫЕ зоны: океан/берег/равнина/горы
    // continental = "насколько это суша" (0 = глубокий океан, 1 = пик горы)
    r.continental = clamp01(continental.octave(wx * 0.001f, wz * 0.001f, 2) * 0.5f + 0.5f);

    // 2. Ридж — реки, обрывы (только в горах!)
    r.ridge = clamp01(continental.ridge(wx * 0.008f, wz * 0.008f, 3));

    // 3. Высота рельефа = continental, но с жёстким water level
    // continental  0.0  → 0.0 (под водой)
    // continental  0.32 → 0.10 (дно, ~6м)
    // continental  0.40 → WATER_LEVEL (берег, ~12м)
    // continental  0.60 → 0.30 (равнина, ~18м)
    // continental  0.80 → 0.55 (горы, ~33м)
    // continental  1.0  → 0.85 (пики, ~51м)
    float terrainH;
    if (r.continental < 0.32f) {
        // Глубокий океан — под водой
        terrainH = r.continental * 0.28f;  // 0..0.09 (0-5м)
    } else if (r.continental < 0.40f) {
        // Берег — подъём к water level
        float t = (r.continental - 0.32f) / 0.08f;
        terrainH = lerp(0.09f, 0.20f, t);  // 5-12м
    } else {
        // СУША — всегда выше water level
        float t = (r.continental - 0.40f) / 0.60f;  // 0..1
        terrainH = lerp(0.20f, 0.85f, t * (2.0f - t));  // 12-51м, ease-out
    }

    // 4. Шум = микротекстура, НЕ холмы!
    float detail = 0.0f;
    float mtn = 0.0f;

    if (r.continental >= 0.40f) {
        // Только на СУШЕ
        mtn = (r.continental - 0.40f) / 0.60f;  // 0 = берег, 1 = пик
    }

    if (r.continental < 0.32f) {
        // Океан — плоский, шум = 0
        detail = 0.0f;
    } else if (r.continental < 0.40f) {
        // Берег — ±0.5 блока
        float freq = 0.02f;
        detail = heightLo.sample(wx * freq, wz * freq) * 0.008f;  // ±0.5м
    } else if (r.continental < 0.70f) {
        // Равнина — ±1-2 блока
        float freq = 0.01f;
        float amp = lerp(0.02f, 0.04f, mtn / 0.5f);  // ±1.2-2.4м
        detail = heightLo.sample(wx * freq, wz * freq) * amp;
    } else {
        // Горы — ±3-8 блоков + обрывы
        float mtnZone = (r.continental - 0.70f) / 0.30f;
        float freq = 0.008f;
        detail = heightLo.sample(wx * freq, wz * freq) * lerp(0.05f, 0.13f, mtnZone);
        detail += r.ridge * 0.06f * mtnZone;  // обрывы
    }

    r.height = clamp01(terrainH + detail);

    // 5. Климат
    r.temperature = clamp01(temperature.octave(wx * 0.002f, wz * 0.002f, 2) * 0.5f + 0.5f);
    r.humidity    = clamp01(humidity.octave(wx * 0.002f, wz * 0.002f, 2) * 0.5f + 0.5f);

    // 6. Биом
    r.type = selectBiome(r.temperature, r.humidity, r.height, r.continental);

    return r;
}

} // namespace
