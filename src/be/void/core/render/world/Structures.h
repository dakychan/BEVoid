#ifndef BEVOID_WORLD_STRUCTURES_H
#define BEVOID_WORLD_STRUCTURES_H

#include "BiomeTypes.h"
#include "Noise.h"
#include "Chunk.h"
#include <vector>
#include <cstdint>

namespace be::void_::core::render::world {

class BiomeNoise;

struct BuildingInstance {
    float x, z;
    float width, depth, height;
    BuildingQuality quality;
};

struct PoleInstance {
    float x, z;
    float height;
    int roadDirection;
};

struct WireInstance {
    float x1, z1, x2, z2;
    float height1, height2;
    float sagAmount;
};

struct BusStopInstance {
    float x, z;
    float width, depth, height;
    BuildingQuality quality;
    bool hasRoof;
    bool hasBench;
};

class StructureGenerator {
public:
    explicit StructureGenerator(uint32_t seed);
    
    void setBiome(const BiomeNoise& biome) { m_biome = &biome; }
    
    void generateStructuresInArea(float startX, float startZ, float endX, float endZ);
    
    void generateDepot(ChunkMesh& mesh, float centerX, float centerZ);
    void generatePole(ChunkMesh& mesh, float x, float groundY, float z, float height);
    void generateWire(ChunkMesh& mesh, float x1, float z1, float y1, float x2, float z2, float y2, float sagAmount);
    void generateWiresInChunk(ChunkMesh& mesh, int chunkCX, int chunkCZ);
    void generateHouse(ChunkMesh& mesh, float x, float z, float w, float d, float h, BuildingQuality q);
    void generateBusStop(ChunkMesh& mesh, float x, float z, BuildingQuality q);
    
    void addBox(ChunkMesh& mesh, 
                float x1, float y1, float z1,
                float x2, float y2, float z2,
                float r, float g, float b);
    
    const std::vector<BuildingInstance>& getBuildings() const { return m_buildings; }
    const std::vector<PoleInstance>& getPoles() const { return m_poles; }
    const std::vector<WireInstance>& getWires() const { return m_wires; }
    const std::vector<BusStopInstance>& getBusStops() const { return m_busStops; }
    
    bool shouldPlacePole(float x, float z) const;
    float calculateWireSag(float x1, float z1, float x2, float z2) const;

private:
    Noise m_structureNoise;
    Noise m_wireSagNoise;
    Noise m_buildingNoise;
    const BiomeNoise* m_biome = nullptr;
    
    std::vector<BuildingInstance> m_buildings;
    std::vector<PoleInstance> m_poles;
    std::vector<WireInstance> m_wires;
    std::vector<BusStopInstance> m_busStops;
};

} // namespace
#endif
