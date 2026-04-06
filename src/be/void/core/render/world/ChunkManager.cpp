#include "ChunkManager.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include <cmath>
#include <algorithm>

namespace be::void_::core::render::world {

/* ---- helpers ---- */
static float lerp(float a, float b, float t) { return a + (b - a) * t; }

static void computeNormal(int x, int z, int grid, const float h[],
                          float& nx, float& ny, float& nz) {
    auto H = [&](int xi, int zi) -> float {
        if (xi < 0 || xi >= grid || zi < 0 || zi >= grid)
            return h[z * grid + x]; // край чанка — не обрыв в 0
        return h[zi * grid + xi];
    };
    float hL = H(x-1, z), hR = H(x+1, z);
    float hD = H(x, z-1), hU = H(x, z+1);
    nx = hL - hR;
    ny = 2.0f;
    nz = hD - hU;
    float len = std::sqrt(nx*nx + ny*ny + nz*nz);
    if (len > 1e-6f) { len = 1.0f / len; } else { nx=0; ny=1; nz=0; return; }
    nx *= len; ny *= len; nz *= len;
}

static BiomeColor blendColor(const BiomeSample& s, const BiomeNoise& biome) {
    BiomeColor c = biomeInfo(s.type).color;
    // 4 соседа
    const float off[4][2] = {{BLEND_RADIUS,0},{-BLEND_RADIUS,0},{0,BLEND_RADIUS},{0,-BLEND_RADIUS}};
    float tr=c.r, tg=c.g, tb=c.b;
    int cnt = 0;
    for (auto& o : off) {
        auto ns = biome.sample(s.wx + o[0], s.wz + o[1]);
        if (ns.type != s.type) {
            auto nc = biomeInfo(ns.type).color;
            tr += nc.r; tg += nc.g; tb += nc.b;
            ++cnt;
        }
    }
    if (cnt > 0) { float w = 1.0f / (cnt+1); tr*=w; tg*=w; tb*=w; }
    return {tr, tg, tb};
}

/* ---- ChunkManager ---- */
ChunkManager::ChunkManager(uint32_t seed)
    : m_seed(seed), m_noise(seed), m_biome(seed) {}

ChunkManager::~ChunkManager() {}

float ChunkManager::terrainHeight(float wx, float wz) const {
    return m_biome.sample(wx, wz).height * MAX_HEIGHT;
}

void ChunkManager::update(float px, float pz, float dt) {
    int pcx = (int)std::floor(px / CHUNK_SIZE);
    int pcz = (int)std::floor(pz / CHUNK_SIZE);
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
    constexpr int G = CHUNK_GRID;
    constexpr int B = CHUNK_SIZE;
    float h[G*G]{};
    ChunkMesh mesh;
    mesh.verts.resize(G*G);
    mesh.idx.reserve(B*B*6);

    for (int z=0; z<G; ++z)
    for (int x=0; x<G; ++x) {
        int i = z*G + x;
        float wx = cx*B + x, wz = cz*B + z;
        auto bs = m_biome.sample(wx, wz);
        float hy = bs.height * MAX_HEIGHT;
        h[i] = hy;

        auto& v = mesh.verts[i];
        v.x = wx; v.y = hy; v.z = wz;
        computeNormal(x, z, G, h, v.nx, v.ny, v.nz);

        BiomeColor col = blendColor(bs, m_biome);
        if (hy > SNOW_LINE) {
            float sf = std::min(1.0f, (hy - SNOW_LINE) / (MAX_HEIGHT - SNOW_LINE));
            v.r = lerp(col.r, 0.95f, sf);
            v.g = lerp(col.g, 0.95f, sf);
            v.b = lerp(col.b, 0.98f, sf);
        } else {
            v.r = col.r; v.g = col.g; v.b = col.b;
        }
    }

    for (int z=0; z<B; ++z)
    for (int x=0; x<B; ++x) {
        uint32_t a=z*G+x, b=a+1, c=(z+1)*G+x, d=c+1;
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
