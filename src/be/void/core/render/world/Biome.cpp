#include "Biome.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>

namespace be::void_::core::render::world {

static float lerp(float a, float b, float t) { return a + (b - a) * t; }
static float clamp01(float x) { return std::max(0.0f, std::min(1.0f, x)); }

static Biome selectBiome(float temp, float hum, float height, float continental) {
    if (continental < 0.32f) return Biome::Shore;
    if (continental < 0.40f) return Biome::Ocean;

    if (height < 0.25f) {
        if (temp < 0.3f)  return Biome::Tundra;
        if (hum < 0.3f)   return Biome::Desert;
        if (hum < 0.6f)   return Biome::Forest;
        return Biome::Swamp;
    }

    if (height < 0.55f) {
        if (hum < 0.3f)   return Biome::Savanna;
        if (hum < 0.6f)   return Biome::Forest;
        return Biome::Jungle;
    }

    if (height > 0.85f) return Biome::Tundra;
    return Biome::Forest;
}

static BuildingQuality selectBuildingQuality(float urbanVal) {
    if (urbanVal < 0.2f)      return BuildingQuality::New;
    if (urbanVal < 0.4f)      return BuildingQuality::Normal;
    if (urbanVal < 0.6f)      return BuildingQuality::Old;
    if (urbanVal < 0.8f)      return BuildingQuality::Ruined;
    return BuildingQuality::Abandoned;
}

BiomeNoise::BiomeNoise(uint32_t s)
    : continental(s)
    , heightLo(s + 100)
    , ridge(s + 300)
    , temperature(s + 1000)
    , humidity(s + 2000)
    , urban(s + 3000)
    , building(s + 4000)
{}

bool BiomeNoise::isNearDepot(float wx, float wz) const {
    float dx = std::abs(wx - DEPOT_SPAWN_X);
    float dz = std::abs(wz - DEPOT_SPAWN_Z);
    return (dx < DEPOT_SIZE_X * 0.6f && dz < DEPOT_SIZE_Z * 0.6f);
}

bool BiomeNoise::isOnRoad(float wx, float wz) const {
    float urbanVal = urban.sample(wx * 0.01f, wz * 0.01f) * 0.5f + 0.5f;
    float roadNoise = urban.sample(wx * 0.05f, wz * 0.05f);
    
    float mainRoadX = std::fmod(std::abs(wx), POLE_SPACING * 2.0f);
    float mainRoadZ = std::fmod(std::abs(wz), POLE_SPACING * 2.0f);
    
    bool onMainRoad = (mainRoadX < ROAD_WIDTH || mainRoadZ < ROAD_WIDTH);
    
    if (isNearDepot(wx, wz)) {
        return true;
    }
    
    return onMainRoad && (urbanVal > 0.3f);
}

BiomeSample BiomeNoise::sample(float wx, float wz) const {
    BiomeSample r;
    r.wx = wx; r.wz = wz;

    r.isDepot = isNearDepot(wx, wz);
    r.isRoad = isOnRoad(wx, wz);

    float urbanVal = urban.sample(wx * 0.01f, wz * 0.01f) * 0.5f + 0.5f;
    float buildingVal = building.sample(wx * 0.02f, wz * 0.02f) * 0.5f + 0.5f;

    r.buildingQuality = selectBuildingQuality(urbanVal);

    if (r.isDepot) {
        r.height = GROUND_LEVEL / MAX_HEIGHT;
        r.type = Biome::Depot;
        r.hasHouse = false;
        r.hasBusStop = false;
        r.continental = 0.5f;
        r.ridge = 0.0f;
        r.temperature = 0.5f;
        r.humidity = 0.5f;
        return r;
    }

    if (r.isRoad) {
        r.height = GROUND_LEVEL / MAX_HEIGHT;
        r.type = Biome::Road;
        r.hasHouse = false;
        r.hasBusStop = false;
        r.continental = 0.5f;
        r.ridge = 0.0f;
        r.temperature = 0.5f;
        r.humidity = 0.5f;
        return r;
    }

    r.continental = clamp01(continental.octave(wx * 0.001f, wz * 0.001f, 2) * 0.5f + 0.5f);
    r.ridge = clamp01(continental.ridge(wx * 0.008f, wz * 0.008f, 3));

    float groundBase = GROUND_LEVEL / MAX_HEIGHT;
    float microHeight = heightLo.sample(wx * 0.05f, wz * 0.05f) * 0.02f;
    r.height = clamp01(groundBase + microHeight);

    r.temperature = clamp01(temperature.octave(wx * 0.002f, wz * 0.002f, 2) * 0.5f + 0.5f);
    r.humidity    = clamp01(humidity.octave(wx * 0.002f, wz * 0.002f, 2) * 0.5f + 0.5f);

    r.hasHouse = (buildingVal < HOUSE_DENSITY) && !r.isRoad && !r.isDepot;
    r.hasBusStop = (buildingVal > 1.0f - BUS_STOP_DENSITY) && r.isRoad && !r.isDepot;

    if (r.hasHouse) {
        r.type = Biome::House;
    } else if (r.hasBusStop) {
        r.type = Biome::BusStop;
    } else if (urbanVal < 0.2f) {
        r.type = Biome::Wasteland;
    } else {
        r.type = selectBiome(r.temperature, r.humidity, r.height, r.continental);
    }

    return r;
}

} // namespace
