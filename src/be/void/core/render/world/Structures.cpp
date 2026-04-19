#include "Structures.h"
#include "Constants.h"
#include "BiomeTypes.h"
#include "Biome.h"
#include <cmath>
#include <algorithm>
#include <set>

namespace be::void_::core::render::world {

StructureGenerator::StructureGenerator(uint32_t seed)
    : m_structureNoise(seed)
    , m_wireSagNoise(seed + 500)
    , m_buildingNoise(seed + 600)
{}

void StructureGenerator::generateStructuresInArea(float startX, float startZ, float endX, float endZ) {
    m_poles.clear();
    m_wires.clear();
    
    for (float z = startZ; z < endZ; z += POLE_SPACING) {
        for (float x = startX; x < endX; x += POLE_SPACING) {
            if (shouldPlacePole(x, z)) {
                PoleInstance pole;
                pole.x = x;
                pole.z = z;
                pole.height = POLE_HEIGHT;
                pole.roadDirection = (int)(x / POLE_SPACING) % 2;
                m_poles.push_back(pole);
            }
        }
    }
    
    std::set<std::pair<size_t, size_t>> wiredPairs;
    
    for (size_t i = 0; i < m_poles.size(); ++i) {
        auto& p1 = m_poles[i];
        
        for (size_t j = i + 1; j < m_poles.size(); ++j) {
            auto& p2 = m_poles[j];
            
            float dx = p2.x - p1.x;
            float dz = p2.z - p1.z;
            float dist = std::sqrt(dx*dx + dz*dz);
            
            if (dist > POLE_SPACING * 1.3f) continue;
            
            bool aligned = false;
            if (std::abs(dx) < 1.0f && std::abs(std::abs(dz) - POLE_SPACING) < 1.0f) aligned = true;
            if (std::abs(dz) < 1.0f && std::abs(std::abs(dx) - POLE_SPACING) < 1.0f) aligned = true;
            
            if (aligned) {
                size_t a = std::min(i, j);
                size_t b = std::max(i, j);
                
                if (wiredPairs.find({a, b}) == wiredPairs.end()) {
                    WireInstance wire;
                    wire.x1 = p1.x;
                    wire.z1 = p1.z;
                    wire.x2 = p2.x;
                    wire.z2 = p2.z;
                    wire.height1 = (float)p1.height;
                    wire.height2 = (float)p2.height;
                    wire.sagAmount = calculateWireSag(p1.x, p1.z, p2.x, p2.z);
                    m_wires.push_back(wire);
                    wiredPairs.insert({a, b});
                }
            }
        }
    }
}

void StructureGenerator::generateWiresInChunk(ChunkMesh& mesh, int chunkCX, int chunkCZ) {
    float chunkMinX = chunkCX * CHUNK_SIZE - POLE_SPACING;
    float chunkMinZ = chunkCZ * CHUNK_SIZE - POLE_SPACING;
    float chunkMaxX = chunkMinX + CHUNK_SIZE + POLE_SPACING;
    float chunkMaxZ = chunkMinZ + CHUNK_SIZE + POLE_SPACING;
    
    for (const auto& wire : m_wires) {
        float midX = (wire.x1 + wire.x2) * 0.5f;
        float midZ = (wire.z1 + wire.z2) * 0.5f;
        
        if (midX >= chunkMinX && midX <= chunkMaxX &&
            midZ >= chunkMinZ && midZ <= chunkMaxZ) {
            generateWire(mesh, wire.x1, wire.z1, wire.height1, wire.x2, wire.z2, wire.height2, wire.sagAmount);
        }
    }
}

bool StructureGenerator::shouldPlacePole(float x, float z) const {
    if (!m_biome) return false;
    
    float depotDist = std::sqrt((x - DEPOT_SPAWN_X) * (x - DEPOT_SPAWN_X) + 
                                (z - DEPOT_SPAWN_Z) * (z - DEPOT_SPAWN_Z));
    if (depotDist < std::max(DEPOT_SIZE_X, DEPOT_SIZE_Z) * 0.5f) {
        return false;
    }
    
    BiomeSample bs = m_biome->sample(x, z);
    if (bs.type != Biome::Road) return false;
    
    float noise = m_structureNoise.sample(x * 0.1f, z * 0.1f) * 0.5f + 0.5f;
    return noise > 0.4f;
}

float StructureGenerator::calculateWireSag(float x1, float z1, float x2, float z2) const {
    float midX = (x1 + x2) * 0.5f;
    float midZ = (z1 + z2) * 0.5f;
    float sagNoise = m_wireSagNoise.sample(midX * 0.5f, midZ * 0.5f) * 0.5f + 0.5f;
    
    float baseSag = WIRE_HEIGHT_NORMAL - WIRE_HEIGHT_MIN;
    return baseSag * (0.2f + sagNoise * 0.8f);
}

void StructureGenerator::generateDepot(ChunkMesh& mesh, float centerX, float centerZ) {
    float halfX = DEPOT_SIZE_X * 0.5f;
    float halfZ = DEPOT_SIZE_Z * 0.5f;
    
    float wallThick = 0.5f;
    float wallH = DEPOT_WALL_HEIGHT;
    
    addBox(mesh, 
           centerX - halfX, 0, centerZ - halfZ,
           centerX + halfX, wallThick, centerZ - halfZ + wallThick,
           0.25f, 0.22f, 0.20f);
    
    addBox(mesh,
           centerX - halfX, 0, centerZ + halfZ - wallThick,
           centerX + halfX, wallThick, centerZ + halfZ,
           0.25f, 0.22f, 0.20f);
    
    addBox(mesh,
           centerX - halfX, 0, centerZ - halfZ,
           centerX - halfX + wallThick, wallH, centerZ + halfZ,
           0.30f, 0.28f, 0.25f);
    
    addBox(mesh,
           centerX + halfX - wallThick, 0, centerZ - halfZ,
           centerX + halfX, wallH, centerZ + halfZ,
           0.30f, 0.28f, 0.25f);
    
    addBox(mesh,
           centerX - halfX, wallH - 0.3f, centerZ - halfZ,
           centerX + halfX, wallH, centerZ + halfZ,
           0.35f, 0.33f, 0.30f);
    
    addBox(mesh,
           centerX - halfX, 0, centerZ - halfZ + wallThick,
           centerX + halfX, 0.1f, centerZ + halfZ - wallThick,
           0.15f, 0.15f, 0.15f);
    
    for (float x = centerX - halfX + POLE_SPACING; x < centerX + halfX; x += POLE_SPACING) {
        generatePole(mesh, x, 0.0f, centerZ - halfZ + 2.0f, POLE_HEIGHT);
        generatePole(mesh, x, 0.0f, centerZ + halfZ - 2.0f, POLE_HEIGHT);
    }
    
    for (float z = centerZ - halfZ + POLE_SPACING; z < centerZ + halfZ; z += POLE_SPACING) {
        generatePole(mesh, centerX - halfX + 2.0f, 0.0f, z, POLE_HEIGHT);
        generatePole(mesh, centerX + halfX - 2.0f, 0.0f, z, POLE_HEIGHT);
    }
    
    for (float x = centerX - halfX + POLE_SPACING; x < centerX + halfX - POLE_SPACING; x += POLE_SPACING) {
        addBox(mesh,
               x - 0.15f, 0.0f, centerZ - halfZ + 2.0f - 0.15f,
               x + 0.15f, POLE_HEIGHT, centerZ - halfZ + 2.0f + 0.15f,
               0.40f, 0.38f, 0.35f);
        
        addBox(mesh,
               x - 0.15f, 0.0f, centerZ + halfZ - 2.0f - 0.15f,
               x + 0.15f, POLE_HEIGHT, centerZ + halfZ - 2.0f + 0.15f,
               0.40f, 0.38f, 0.35f);
    }
}

void StructureGenerator::generatePole(ChunkMesh& mesh, float x, float groundY, float z, float height) {
    float poleRadius = 0.15f;
    float topY = groundY + height;
    
    addBox(mesh,
           x - poleRadius, groundY, z - poleRadius,
           x + poleRadius, topY, z + poleRadius,
           0.40f, 0.38f, 0.35f);
    
    addBox(mesh,
           x - 0.8f, topY - 0.2f, z - 0.1f,
           x + 0.8f, topY, z + 0.1f,
           0.35f, 0.33f, 0.30f);
    
    addBox(mesh,
           x - 0.1f, topY - 0.2f, z - 0.8f,
           x + 0.1f, topY, z + 0.8f,
           0.35f, 0.33f, 0.30f);
}

void StructureGenerator::generateWire(ChunkMesh& mesh, float x1, float z1, float y1, float x2, float z2, float y2, float sagAmount) {
    ChunkWire wire;
    wire.x1 = x1;
    wire.y1 = y1;
    wire.z1 = z1;
    wire.x2 = x2;
    wire.y2 = y2;
    wire.z2 = z2;
    wire.sag = sagAmount;
    mesh.wires.push_back(wire);
}

void StructureGenerator::generateHouse(ChunkMesh& mesh, 
                                        float x, float z, float w, float d, float h,
                                        BuildingQuality q) {
    BiomeColor col = buildingColor(q);
    
    addBox(mesh,
           x - w * 0.5f, 0, z - d * 0.5f,
           x + w * 0.5f, h, z + d * 0.5f,
           col.r, col.g, col.b);
    
    float roofH = h * 0.3f;
    addBox(mesh,
           x - w * 0.55f, h, z - d * 0.55f,
           x + w * 0.55f, h + roofH, z + d * 0.55f,
           col.r * 0.7f, col.g * 0.65f, col.b * 0.6f);
    
    float winW = 0.8f;
    float winH = 1.0f;
    float winY = h * 0.3f;
    
    addBox(mesh,
           x - w * 0.5f - 0.05f, winY, z - winW * 0.5f,
           x - w * 0.5f, winY + winH, z + winW * 0.5f,
           0.3f, 0.5f, 0.7f);
    
    addBox(mesh,
           x + w * 0.5f, winY, z - winW * 0.5f,
           x + w * 0.5f + 0.05f, winY + winH, z + winW * 0.5f,
           0.3f, 0.5f, 0.7f);
}

void StructureGenerator::generateBusStop(ChunkMesh& mesh, float x, float z, BuildingQuality q) {
    BiomeColor col = buildingColor(q);
    
    float w = 4.0f;
    float d = 2.0f;
    float h = 3.0f;
    float roofH = 0.3f;
    
    addBox(mesh,
           x - w * 0.5f, 0, z - d * 0.5f,
           x + w * 0.5f, h, z + d * 0.5f,
           col.r, col.g, col.b);
    
    addBox(mesh,
           x - w * 0.7f, h, z - d * 0.7f,
           x + w * 0.7f, h + roofH, z + d * 0.7f,
           col.r * 0.8f, col.g * 0.8f, col.b * 0.8f);
    
    float benchW = 1.5f;
    float benchH = 0.5f;
    float benchD = 0.4f;
    
    addBox(mesh,
           x - benchW * 0.5f, 0, z + d * 0.5f - benchD,
           x + benchW * 0.5f, benchH, z + d * 0.5f,
           0.35f, 0.25f, 0.15f);
}

void StructureGenerator::addBox(ChunkMesh& mesh,
                                 float x1, float y1, float z1,
                                 float x2, float y2, float z2,
                                 float r, float g, float b) {
    uint32_t baseIdx = (uint32_t)mesh.verts.size();
    
    mesh.verts.push_back({x1, y1, z1, 0, -1, 0, r, g, b});
    mesh.verts.push_back({x2, y1, z1, 0, -1, 0, r, g, b});
    mesh.verts.push_back({x2, y1, z2, 0, -1, 0, r, g, b});
    mesh.verts.push_back({x1, y1, z2, 0, -1, 0, r, g, b});
    
    mesh.verts.push_back({x1, y2, z1, 0, 1, 0, r, g, b});
    mesh.verts.push_back({x2, y2, z1, 0, 1, 0, r, g, b});
    mesh.verts.push_back({x2, y2, z2, 0, 1, 0, r, g, b});
    mesh.verts.push_back({x1, y2, z2, 0, 1, 0, r, g, b});
    
    mesh.verts.push_back({x1, y1, z1, -1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x1, y2, z1, -1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x1, y2, z2, -1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x1, y1, z2, -1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    
    mesh.verts.push_back({x2, y1, z1, 1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x2, y2, z1, 1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x2, y2, z2, 1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    mesh.verts.push_back({x2, y1, z2, 1, 0, 0, r * 0.9f, g * 0.9f, b * 0.9f});
    
    mesh.verts.push_back({x1, y1, z1, 0, 0, -1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x2, y1, z1, 0, 0, -1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x2, y2, z1, 0, 0, -1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x1, y2, z1, 0, 0, -1, r * 0.8f, g * 0.8f, b * 0.8f});
    
    mesh.verts.push_back({x1, y1, z2, 0, 0, 1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x2, y1, z2, 0, 0, 1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x2, y2, z2, 0, 0, 1, r * 0.8f, g * 0.8f, b * 0.8f});
    mesh.verts.push_back({x1, y2, z2, 0, 0, 1, r * 0.8f, g * 0.8f, b * 0.8f});
    
    auto addQuad = [&](uint32_t a, uint32_t b, uint32_t c, uint32_t d) {
        mesh.idx.push_back(baseIdx + a);
        mesh.idx.push_back(baseIdx + b);
        mesh.idx.push_back(baseIdx + c);
        mesh.idx.push_back(baseIdx + a);
        mesh.idx.push_back(baseIdx + c);
        mesh.idx.push_back(baseIdx + d);
    };
    
    addQuad(0, 1, 2, 3);
    addQuad(4, 5, 6, 7);
    addQuad(8, 9, 10, 11);
    addQuad(12, 13, 14, 15);
    addQuad(16, 17, 18, 19);
    addQuad(20, 23, 22, 21);
}

} // namespace
