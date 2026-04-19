#ifndef BEVOID_WORLD_BIOME_H
#define BEVOID_WORLD_BIOME_H

#include "BiomeTypes.h"
#include "Noise.h"

namespace be::void_::core::render::world {

struct BiomeSample {
    float height;
    float continental;
    float ridge;
    float temperature;
    float humidity;
    float wx, wz;
    Biome type;
    BuildingQuality buildingQuality;
    bool isDepot;
    bool isRoad;
    bool hasBusStop;
    bool hasHouse;
};

class BiomeNoise {
public:
    explicit BiomeNoise(uint32_t seed);
    BiomeSample sample(float wx, float wz) const;
    
    bool isNearDepot(float wx, float wz) const;
    bool isOnRoad(float wx, float wz) const;

private:
    Noise continental;
    Noise heightLo;
    Noise ridge;
    Noise temperature;
    Noise humidity;
    Noise urban;
    Noise building;
};

} // namespace
#endif
