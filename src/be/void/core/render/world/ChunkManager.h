#ifndef BEVOID_WORLD_CHUNK_MANAGER_H
#define BEVOID_WORLD_CHUNK_MANAGER_H

#include "Chunk.h"
#include "Biome.h"
#include "Structures.h"
#include <unordered_map>
#include <vector>
#include <atomic>
#include <memory>

namespace be::void_::core::render::world {

class ChunkManager {
public:
    explicit ChunkManager(uint32_t seed = 77777);
    ~ChunkManager();

    void update(float px, float pz, float dt);
    void draw() const;
    void drawWires(const float* viewMat, const float* projMat, float camX, float camY, float camZ, float time) const;
    float terrainHeight(float wx, float wz) const;

    void setSeed(uint32_t seed);
    uint32_t getSeed() const { return m_seed; }

    void setRenderDistance(int r) { m_rdist = r; }
    int  renderDistance() const   { return m_rdist; }
    
    StructureGenerator& getStructures() { return m_structures; }

private:
    struct Key {
        int x, z;
        bool operator==(const Key& o) const { return x==o.x && z==o.z; }
    };
    struct KeyHash {
        size_t operator()(Key k) const {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.z) + 0x9e3779b9 + (std::hash<int>()(k.x) << 6) + (std::hash<int>()(k.x) >> 2));
        }
    };

    void buildChunk(int cx, int cz);
    void flushPending();
    void generateChunkStructures(ChunkMesh& mesh, int cx, int cz);

    uint32_t m_seed;
    int      m_rdist = 8;
    BiomeNoise m_biome;
    StructureGenerator m_structures;

    std::unordered_map<Key, std::unique_ptr<Chunk>, KeyHash> m_chunks;

    int    m_lastCx = 0x7FFFFFFF, m_lastCz = 0x7FFFFFFF;
    float  m_acc = 0;
    
    bool   m_depotGenerated = false;
    bool   m_structuresGenerated = false;
};

} // namespace
#endif
