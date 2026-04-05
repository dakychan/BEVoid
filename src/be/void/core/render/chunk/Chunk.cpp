/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

/*
 * be.void.core.render.chunk — Chunk implementation
 *
 * Генерирует terrain mesh из heightmap (Simplex noise).
 */

#include "core/render/chunk/Chunk.h"
#include <cmath>

namespace be::void_::core::render::chunk {

static constexpr float TERRAIN_HEIGHT = 12.0f;  /* максимальная высота */
static constexpr float TERRAIN_SCALE  = 0.04f;  /* меньше = плавнее рельеф */

Chunk::Chunk(int cx, int cz, uint32_t seed)
    : m_cx(cx), m_cz(cz), m_seed(seed) {}

Chunk::~Chunk() {
    unloadGL();
}

void Chunk::generate(const Noise& noise) {
    if (m_generating.exchange(true)) return; /* уже генерируется */

    /* --- Heightmap из Simplex noise --- */
    float offsetX = m_cx * CHUNK_SIZE * TERRAIN_SCALE;
    float offsetZ = m_cz * CHUNK_SIZE * TERRAIN_SCALE;

    for (int z = 0; z < HEIGHT_MAP; z++) {
        for (int x = 0; x < HEIGHT_MAP; x++) {
            float wx = offsetX + x * TERRAIN_SCALE;
            float wz = offsetZ + z * TERRAIN_SCALE;
            float n = noise.octaveNoise2D(wx, wz, 4);
            /* Нормализуем [0, 1] → [0, TERRAIN_HEIGHT] */
            m_heightmap[z * HEIGHT_MAP + x] = (n * 0.5f + 0.5f) * TERRAIN_HEIGHT;
        }
    }

    /* --- Вершины --- */
    m_vertices.clear();
    m_indices.clear();
    m_vertices.reserve(HEIGHT_MAP * HEIGHT_MAP);
    m_indices.reserve(CHUNK_SIZE * CHUNK_SIZE * 6);

    for (int z = 0; z < HEIGHT_MAP; z++) {
        for (int x = 0; x < HEIGHT_MAP; x++) {
            ChunkVertex v;
            v.x = m_cx * CHUNK_SIZE + x;
            v.y = m_heightmap[z * HEIGHT_MAP + x];
            v.z = m_cz * CHUNK_SIZE + z;

            /* Нормаль — approximate из соседей */
            float hL = (x > 0) ? m_heightmap[z * HEIGHT_MAP + (x-1)] : v.y;
            float hR = (x < CHUNK_SIZE) ? m_heightmap[z * HEIGHT_MAP + (x+1)] : v.y;
            float hD = (z > 0) ? m_heightmap[(z-1) * HEIGHT_MAP + x] : v.y;
            float hU = (z < CHUNK_SIZE) ? m_heightmap[(z+1) * HEIGHT_MAP + x] : v.y;

            float nx = hL - hR;
            float ny = 2.0f;
            float nz = hD - hU;
            float len = std::sqrt(nx*nx + ny*ny + nz*nz);
            v.nx = nx/len; v.ny = ny/len; v.nz = nz/len;

            /* Цвет по высоте: зелёный низ, серый верх, белый пики */
            float t = v.y / TERRAIN_HEIGHT;
            if (t < 0.3f) {
                v.r = 0.15f + t*0.5f; v.g = 0.5f + t*0.5f; v.b = 0.1f;
            } else if (t < 0.6f) {
                v.r = 0.4f + t*0.3f; v.g = 0.35f;           v.b = 0.15f;
            } else if (t < 0.85f) {
                v.r = 0.5f + t*0.3f; v.g = 0.5f + t*0.2f;   v.b = 0.5f + t*0.2f;
            } else {
                v.r = 0.95f; v.g = 0.95f; v.b = 0.98f; /* снег */
            }

            m_vertices.push_back(v);
        }
    }

    /* --- Индексы (triangles) --- */
    for (int z = 0; z < CHUNK_SIZE; z++) {
        for (int x = 0; x < CHUNK_SIZE; x++) {
            uint32_t a = z * HEIGHT_MAP + x;
            uint32_t b = a + 1;
            uint32_t c = (z+1) * HEIGHT_MAP + x;
            uint32_t d = c + 1;
            m_indices.push_back(a); m_indices.push_back(c); m_indices.push_back(b);
            m_indices.push_back(b); m_indices.push_back(c); m_indices.push_back(d);
        }
    }

    m_generated = true;
    m_generating = false;
}

void Chunk::loadGL() {
    if (!m_generated || m_glLoaded) return;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(ChunkVertex),
                 m_vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint32_t),
                 m_indices.data(), GL_STATIC_DRAW);

    /* attrib 0: vec3 position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)0);

    /* attrib 1: vec3 normal */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)12);

    /* attrib 2: vec3 color */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ChunkVertex), (void*)24);

    glBindVertexArray(0);
    m_glLoaded = true;
}

void Chunk::draw() const {
    if (!m_glLoaded) return;
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()),
                   GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Chunk::unloadGL() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    m_vao = 0; m_vbo = 0; m_ebo = 0;
    m_glLoaded = false;
}

} // namespace be::void_::core::render::chunk
