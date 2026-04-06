#ifndef BEVOID_WORLD_BIOME_H
#define BEVOID_WORLD_BIOME_H

#include "BiomeTypes.h"
#include "Noise.h"

namespace be::void_::core::render::world {

struct BiomeSample {
    float height;       // 0..1
    float temperature;  // 0..1
    float humidity;     // 0..1
    float wx, wz;       // мировые координаты
    Biome type;
};

class BiomeNoise {
public:
    explicit BiomeNoise(uint32_t seed);
    BiomeSample sample(float wx, float wz) const;

private:
    Noise heightNoise;
    Noise tempNoise;
    Noise humidityNoise;
};

} // namespace
#endif
