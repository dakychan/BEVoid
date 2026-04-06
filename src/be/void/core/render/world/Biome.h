#ifndef BEVOID_WORLD_BIOME_H
#define BEVOID_WORLD_BIOME_H

#include "BiomeTypes.h"
#include "Noise.h"

namespace be::void_::core::render::world {

struct BiomeSample {
    float height;       // 0..1 — рельеф
    float continental;  // 0..1 — зона: океан/равнина/горы
    float ridge;        // 0..1 — реки/обрывы
    float temperature;  // 0..1 — большие климатические зоны
    float humidity;     // 0..1 — влажность
    float wx, wz;
    Biome type;
};

class BiomeNoise {
public:
    explicit BiomeNoise(uint32_t seed);
    BiomeSample sample(float wx, float wz) const;

private:
    Noise continental;  // 0.001 — океан/равнина/горы
    Noise heightLo;     // микротекстура
    Noise ridge;        // 0.008 — реки/обрывы
    Noise temperature;  // 0.002 — климат
    Noise humidity;     // 0.002 — влажность
};

} // namespace
#endif
