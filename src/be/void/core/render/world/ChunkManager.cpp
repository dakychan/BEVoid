#include "ChunkManager.h"
#include "Biome.h"
#include "BiomeTypes.h"
#include "Constants.h"
#include "Noise.h"
#include "Structures.h"
#include <cmath>
#include <algorithm>
#include <memory>

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

static float lerp(float a, float b, float t) { return a + (b - a) * t; }

ChunkManager::ChunkManager(uint32_t seed)
    : m_seed(seed)
    , m_biome(seed)
    , m_structures(seed)
{
    m_structures.setBiome(m_biome);
}

ChunkManager::~ChunkManager() {
}

void ChunkManager::setSeed(uint32_t seed) {
    m_seed = seed;
    m_biome = BiomeNoise(seed);
    m_structures = StructureGenerator(seed);
    m_structures.setBiome(m_biome);
    for (auto& [k, c] : m_chunks) {
        if (c) c->unload();
    }
    m_chunks.clear();
    m_lastCx = 0x7FFFFFFF;
    m_lastCz = 0x7FFFFFFF;
    m_depotGenerated = false;
    m_structuresGenerated = false;
    m_acc = 0;
    LOGI("[ChunkManager] Seed changed to %u\n", seed);
}

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
    
    if (!m_structuresGenerated) {
        int genRange = 50;
        m_structures.generateStructuresInArea(
            (pcx - genRange) * CHUNK_SIZE,
            (pcz - genRange) * CHUNK_SIZE,
            (pcx + genRange) * CHUNK_SIZE,
            (pcz + genRange) * CHUNK_SIZE
        );
        m_structuresGenerated = true;
        LOGI("[ChunkManager] Structures generated\n");
    }
    
    m_acc += dt;
    if (m_acc < 0.5f) { flushPending(); return; }
    m_acc = 0;
    flushPending();

    if (pcx == m_lastCx && pcz == m_lastCz) return;
    m_lastCx = pcx; m_lastCz = pcz;

    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        int dx = it->first.x - pcx, dz = it->first.z - pcz;
        if (std::abs(dx) + std::abs(dz) > m_rdist + 1) {
            it->second->unload();
            it = m_chunks.erase(it);
        } else ++it;
    }

    for (int dz = -m_rdist; dz <= m_rdist; ++dz)
    for (int dx = -m_rdist; dx <= m_rdist; ++dx) {
        int cx = pcx+dx, cz = pcz+dz;
        Key k{cx, cz};
        if (m_chunks.count(k)) continue;

        buildChunk(cx, cz);
    }
}

void ChunkManager::generateChunkStructures(ChunkMesh& mesh, int cx, int cz) {
    float chunkStartX = (float)(cx * CHUNK_SIZE);
    float chunkStartZ = (float)(cz * CHUNK_SIZE);
    float chunkEndX = chunkStartX + CHUNK_SIZE;
    float chunkEndZ = chunkStartZ + CHUNK_SIZE;
    
    if (!m_depotGenerated && 
        chunkStartX <= DEPOT_SPAWN_X && chunkEndX >= DEPOT_SPAWN_X &&
        chunkStartZ <= DEPOT_SPAWN_Z && chunkEndZ >= DEPOT_SPAWN_Z) {
        m_structures.generateDepot(mesh, DEPOT_SPAWN_X, DEPOT_SPAWN_Z);
        m_depotGenerated = true;
        LOGI("[ChunkManager] Depot generated at spawn\n");
    }
    
    m_structures.generateWiresInChunk(mesh, cx, cz);
    
    for (float x = chunkStartX; x < chunkEndX; x += POLE_SPACING) {
        for (float z = chunkStartZ; z < chunkEndZ; z += POLE_SPACING) {
            auto bs = m_biome.sample(x, z);
            
            if (bs.isRoad && !bs.isDepot) {
                float depotDist = std::sqrt((x - DEPOT_SPAWN_X) * (x - DEPOT_SPAWN_X) + 
                                            (z - DEPOT_SPAWN_Z) * (z - DEPOT_SPAWN_Z));
                if (depotDist > std::max(DEPOT_SIZE_X, DEPOT_SIZE_Z) * 0.5f) {
                    m_structures.generatePole(mesh, x, 0.0f, z, POLE_HEIGHT);
                }
            }
            
            if (bs.hasHouse && !bs.isDepot && !bs.isRoad) {
                float houseSize = HOUSE_MIN_SIZE + 
                    (HOUSE_MAX_SIZE - HOUSE_MIN_SIZE) * 
                    (m_structures.shouldPlacePole(x, z) ? 0.5f : 0.3f);
                float houseH = HOUSE_MIN_HEIGHT + 
                    (HOUSE_MAX_HEIGHT - HOUSE_MIN_HEIGHT) * 
                    (m_structures.shouldPlacePole(x + 100, z + 100) ? 0.6f : 0.3f);
                m_structures.generateHouse(mesh, x, z, houseSize, houseSize, houseH, bs.buildingQuality);
            }
            
            if (bs.hasBusStop && bs.isRoad && !bs.isDepot) {
                m_structures.generateBusStop(mesh, x, z, bs.buildingQuality);
            }
        }
    }
}

void ChunkManager::buildChunk(int cx, int cz) {
    constexpr int PAD = 1;
    constexpr int G = CHUNK_GRID + 2;
    constexpr int B = CHUNK_SIZE;

    float h[G*G]{};
    for (int z=0; z<G; ++z)
    for (int x=0; x<G; ++x) {
        float wx = static_cast<float>((cx - PAD) * B + x);
        float wz = static_cast<float>((cz - PAD) * B + z);
        h[z*G + x] = m_biome.sample(wx, wz).height * MAX_HEIGHT;
    }

    ChunkMesh mesh;
    int VERT_G = CHUNK_GRID;
    mesh.verts.resize(VERT_G * VERT_G);
    mesh.idx.reserve(B * B * 6);

    for (int z = 0; z < VERT_G; ++z)
    for (int x = 0; x < VERT_G; ++x) {
        int i = z * VERT_G + x;

        float wx = static_cast<float>(cx * B + x);
        float wz = static_cast<float>(cz * B + z);
        auto bs = m_biome.sample(wx, wz);
        float hy = bs.height * MAX_HEIGHT;

        int px = x + PAD;
        int pz = z + PAD;

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
        
        v.r = col.r; v.g = col.g; v.b = col.b;
        v.u = wx * 0.1f;
        v.v = wz * 0.1f;
    }

    for (int z = 0; z < B; ++z)
    for (int x = 0; x < B; ++x) {
        uint32_t a = z*VERT_G + x;
        uint32_t b = a + 1;
        uint32_t c = (z+1)*VERT_G + x;
        uint32_t d = c + 1;
        mesh.idx.push_back(c); mesh.idx.push_back(b); mesh.idx.push_back(a);
        mesh.idx.push_back(c); mesh.idx.push_back(d); mesh.idx.push_back(b);
    }

    generateChunkStructures(mesh, cx, cz);

    auto chunk = std::unique_ptr<Chunk>(new Chunk(cx, cz));
    chunk->load(mesh);
    m_chunks[Key{cx,cz}] = std::move(chunk);
}

void ChunkManager::flushPending() {
}

void ChunkManager::draw() const {
    for (auto& [k, c] : m_chunks)
        if (c && c->loaded()) c->draw();
}

void ChunkManager::drawWires(const float* viewMat, const float* projMat, float camX, float camY, float camZ, float time) const {
    static GLuint wireProg = 0;
    static GLint uView = -1;
    static GLint uProj = -1;
    
    if (!wireProg) {
#if defined(BEVOID_PLATFORM_ANDROID)
        const char* vs =
            "#version 300 es\n"
            "layout(location = 0) in vec3 aPos;\n"
            "uniform mat4 uView;\n"
            "uniform mat4 uProj;\n"
            "void main() {\n"
            "    gl_Position = uProj * uView * vec4(aPos, 1.0);\n"
            "}\n";

        const char* fs =
            "#version 300 es\n"
            "precision mediump float;\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    fragColor = vec4(0.05, 0.05, 0.05, 1.0);\n"
            "}\n";
#else
        const char* vs = 
            "#version 330 core\n"
            "layout(location = 0) in vec3 aPos;\n"
            "uniform mat4 uView;\n"
            "uniform mat4 uProj;\n"
            "void main() {\n"
            "    gl_Position = uProj * uView * vec4(aPos, 1.0);\n"
            "}\n";
        
        const char* fs =
            "#version 330 core\n"
            "out vec4 fragColor;\n"
            "void main() {\n"
            "    fragColor = vec4(0.05, 0.05, 0.05, 1.0);\n"
            "}\n";
#endif
        
        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &vs, nullptr);
        glCompileShader(v);
        
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &fs, nullptr);
        glCompileShader(f);
        
        wireProg = glCreateProgram();
        glAttachShader(wireProg, v);
        glAttachShader(wireProg, f);
        glLinkProgram(wireProg);
        
        glDeleteShader(v);
        glDeleteShader(f);
        
        uView = glGetUniformLocation(wireProg, "uView");
        uProj = glGetUniformLocation(wireProg, "uProj");
    }
    
    glUseProgram(wireProg);
    glUniformMatrix4fv(uView, 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(uProj, 1, GL_FALSE, projMat);
    
    std::vector<float> wireVerts;
    struct WireDraw { int startVert; int vertCount; };
    std::vector<WireDraw> wireDraws;
    
    for (auto& [k, c] : m_chunks) {
        if (!c || !c->loaded()) continue;
        
        const auto& wires = c->getWires();
        if (wires.empty()) continue;
        
        for (const auto& w : wires) {
            float dx = w.x2 - w.x1;
            float dz = w.z2 - w.z1;
            float len = std::sqrt(dx*dx + dz*dz);
            if (len < 0.001f) continue;
            
            float nx = dx / len;
            float nz = dz / len;
            float offX = -nz * 0.5f;
            float offZ = nx * 0.5f;
            
            int segments = 30;
            float dy = w.y2 - w.y1;
            
            for (int side = 0; side < 2; ++side) {
                float soff = (side == 0) ? -0.51f : 0.51f;
                int startVert = (int)(wireVerts.size() / 3);
                
                for (int i = 0; i <= segments; ++i) {
                    float t = (float)i / (float)segments;
                    float x = w.x1 + dx * t + offX * soff;
                    float z = w.z1 + dz * t + offZ * soff;
                    float y = w.y1 + dy * t - w.sag * std::sin(t * 3.14159f);
                    
                    float wind = std::sin(time * 1.2f + x * 0.4f + z * 0.3f) * 0.03f;
                    y += wind;
                    
                    wireVerts.push_back(x);
                    wireVerts.push_back(y);
                    wireVerts.push_back(z);
                }
                
                wireDraws.push_back({startVert, segments + 1});
            }
        }
    }
    
    if (wireVerts.empty()) return;
    
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, wireVerts.size() * sizeof(float), wireVerts.data(), GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glLineWidth(1.5f);
    
    for (const auto& wd : wireDraws) {
        glDrawArrays(GL_LINE_STRIP, wd.startVert, wd.vertCount);
    }
    
    glLineWidth(1.0f);
    
    glBindVertexArray(0);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    
    glUseProgram(0);
}

} // namespace
