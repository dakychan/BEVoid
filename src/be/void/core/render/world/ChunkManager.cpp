#include "ChunkManager.h"
#include "Biome.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include "Noise.h"
#include <cmath>
#include <algorithm>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <android/log.h>
    #define LOG_TAG "BEVoid"
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
    #include <cstdio>
    #define LOGI(...) std::printf(__VA_ARGS__)
    #define LOGE(...) std::fprintf(stderr, __VA_ARGS__)
#endif

namespace be::void_::core::render::world {

/* ---- helpers ---- */
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

/* ---- ChunkManager ---- */
ChunkManager::ChunkManager(uint32_t seed)
    : m_seed(seed), m_biome(seed) {}

ChunkManager::~ChunkManager() {}

float ChunkManager::terrainHeight(float wx, float wz) const {
    return m_biome.sample(wx, wz).height * MAX_HEIGHT;
}

void ChunkManager::update(float px, float pz, float dt) {
    int pcx = (int)std::floor(px / CHUNK_SIZE);
    int pcz = (int)std::floor(pz / CHUNK_SIZE);
    static int lastLogCx = 0x7FFFFFFF, lastLogCz = 0x7FFFFFFF;
    if (pcx != lastLogCx || pcz != lastLogCz) {
        LOGI("[ChunkManager] Player at chunk (%d,%d) pos (%.1f,%.1f)\n", pcx, pcz, px, pz);
        lastLogCx = pcx; lastLogCz = pcz;
    }
    m_acc += dt;
    if (m_acc < 0.5f) { flushPending(); return; }
    m_acc = 0;
    flushPending();

    if (pcx == m_lastCx && pcz == m_lastCz) return;
    m_lastCx = pcx; m_lastCz = pcz;

    // unload far
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        int dx = it->first.x - pcx, dz = it->first.z - pcz;
        if (std::abs(dx) + std::abs(dz) > m_rdist + 1) {
            it->second->unload();
            it = m_chunks.erase(it);
        } else ++it;
    }

    // request near
    for (int dz = -m_rdist; dz <= m_rdist; ++dz)
    for (int dx = -m_rdist; dx <= m_rdist; ++dx) {
        int cx = pcx+dx, cz = pcz+dz;
        Key k{cx, cz};
        if (m_chunks.count(k)) continue;
        // check pending
        bool inP = false;
        for (auto& p : m_pending) if (p.cx==cx && p.cz==cz) { inP=true; break; }
        if (inP) continue;

        if (std::abs(dx)+std::abs(dz) <= 1) {
            buildChunk(cx, cz);          // sync — ближние
        } else {
            // async — дальние (заглушка, пока без потока)
            m_pending.push_back({cx, cz, ChunkMesh{}});
            auto& pm = m_pending.back().mesh;
            // временно синхронно:
            buildChunk(cx, cz);
            m_pending.pop_back();
        }
    }
}

void ChunkManager::buildChunk(int cx, int cz) {
    // Генерируем 19x19 вершин (16 + 1 блок с каждой стороны) для корректных нормалей
    constexpr int PAD = 1;
    constexpr int G = CHUNK_GRID + 2;  // 19
    constexpr int B = CHUNK_SIZE;       // 16

    // Собираем высоты с оверлапом
    float h[G*G]{};
    for (int z=0; z<G; ++z)
    for (int x=0; x<G; ++x) {
        float wx = static_cast<float>((cx - PAD) * B + x);  // мировые координаты с паддингом
        float wz = static_cast<float>((cz - PAD) * B + z);
        h[z*G + x] = m_biome.sample(wx, wz).height * MAX_HEIGHT;
    }

    // Строим вершины только для центральных 16x16 (индексы 1..17)
    ChunkMesh mesh;
    int VERT_G = CHUNK_GRID;  // 17 вершин на 16 блоков
    mesh.verts.resize(VERT_G * VERT_G);
    mesh.idx.reserve(B * B * 6);

    for (int z = 0; z < VERT_G; ++z)
    for (int x = 0; x < VERT_G; ++x) {
        int i = z * VERT_G + x;

        float wx = static_cast<float>(cx * B + x);
        float wz = static_cast<float>(cz * B + z);
        auto bs = m_biome.sample(wx, wz);
        float hy = bs.height * MAX_HEIGHT;

        // Вершина (x,z) в padded массиве = (x+1, z+1)
        int px = x + PAD;
        int pz = z + PAD;

        // Нормаль из padded-соседей
        float hL = h[pz * G + (px - 1)];
        float hR = h[pz * G + (px + 1)];
        float hD = h[(pz - 1) * G + px];
        float hU = h[(pz + 1) * G + px];
        float nx = hL - hR;
        float ny = 2.0f;
        float nz = hD - hU;
        float len = std::sqrt(nx*nx + ny*ny + nz*nz);
        if (len > 1e-6f) { len = 1.0f / len; } else { nx=0; ny=1; nz=0; }
        nx *= len; ny *= len; nz *= len;

        static thread_local Noise colorVar(99999);
        float cv = colorVar.sample(wx * 0.05f, wz * 0.05f);
        BiomeColor col = biomeColor(bs.type, bs.height, bs.humidity, bs.ridge, cv);

        auto& v = mesh.verts[i];
        v.x = wx; v.y = hy; v.z = wz;
        v.nx = nx; v.ny = ny; v.nz = nz;
        if (hy > SNOW_LINE) {
            float sf = std::min(1.0f, (hy - SNOW_LINE) / (MAX_HEIGHT - SNOW_LINE));
            v.r = lerp(col.r, 0.95f, sf);
            v.g = lerp(col.g, 0.95f, sf);
            v.b = lerp(col.b, 0.98f, sf);
        } else {
            v.r = col.r; v.g = col.g; v.b = col.b;
        }
    }

    // Индексы
    for (int z = 0; z < B; ++z)
    for (int x = 0; x < B; ++x) {
        uint32_t a = z*VERT_G + x;
        uint32_t b = a + 1;
        uint32_t c = (z+1)*VERT_G + x;
        uint32_t d = c + 1;
        mesh.idx.push_back(c); mesh.idx.push_back(b); mesh.idx.push_back(a);
        mesh.idx.push_back(c); mesh.idx.push_back(d); mesh.idx.push_back(b);
    }

    auto chunk = std::make_unique<Chunk>(cx, cz);
    chunk->load(mesh);
    m_chunks[Key{cx,cz}] = std::move(chunk);
}

void ChunkManager::flushPending() {
    for (auto& p : m_pending) {
        Key k{p.cx, p.cz};
        if (!m_chunks.count(k)) {
            // если ещё не построен — строить
        }
    }
    m_pending.clear();
}

void ChunkManager::draw() const {
    for (auto& [k, c] : m_chunks)
        if (c && c->loaded()) c->draw();
}

} // namespace
