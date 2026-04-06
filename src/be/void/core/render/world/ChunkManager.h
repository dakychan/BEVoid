#ifndef BEVOID_WORLD_CHUNK_MANAGER_H
#define BEVOID_WORLD_CHUNK_MANAGER_H

#include "Chunk.h"
#include "Biome.h"
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

namespace be::void_::core::render::world {

class ChunkManager {
public:
    explicit ChunkManager(uint32_t seed = 77777);
    ~ChunkManager();

    void update(float px, float pz, float dt);
    void draw() const;
    float terrainHeight(float wx, float wz) const;

    void setRenderDistance(int r) { m_rdist = r; }
    int  renderDistance() const   { return m_rdist; }

private:
    struct Key {
        int x, z;
        bool operator==(const Key& o) const { return x==o.x && z==o.z; }
    };
    struct KeyHash {
        size_t operator()(Key k) const {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.z) << 16);
        }
    };

    struct Pending {
        int cx, cz;
        ChunkMesh mesh;
    };

    void buildChunk(int cx, int cz);
    void flushPending();

    uint32_t m_seed;
    int      m_rdist = 4;
    BiomeNoise m_biome;

    std::unordered_map<Key, std::unique_ptr<Chunk>, KeyHash> m_chunks;
    std::vector<Pending> m_pending;
    std::mutex           m_mtx;

    int    m_lastCx = 0x7FFFFFFF, m_lastCz = 0x7FFFFFFF;
    float  m_acc = 0;
};

} // namespace
#endif
