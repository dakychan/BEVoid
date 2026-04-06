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
  * be.void.core.render.world.chunk — Chunk implementation
  *
  * Генерирует terrain mesh из heightmap с биомами.
  */

#include "Chunk.h"
#include "../biome/BiomeNoise.h"
#include <cmath>

namespace be::void_::core::render::world::chunk {

    static constexpr float TERRAIN_HEIGHT = 200.0f;  /* максимальная высота 200м */

    Chunk::Chunk(int cx, int cz, uint32_t seed)
        : m_cx(cx), m_cz(cz), m_seed(seed) {
    }

    Chunk::~Chunk() {
        unloadGL();
    }

    void Chunk::generate(const Noise& noise, const biome::BiomeNoise& biomeNoise) {
        if (m_generating.exchange(true)) return; /* уже генерируется */

        /*
         * ФИКС: Четко определяем размерность сетки.
         * Для покрытия квадрата 16x16 блоков нужна сетка 17x17 вершин.
         */
        const int GRID_SIZE = 17;
        const int VERT_COUNT = GRID_SIZE * GRID_SIZE;

        m_vertices.resize(VERT_COUNT);
        m_indices.clear();

        /* Резервируем память под индексы: 16x16 квадратов = 256 квадратов * 2 треугольника * 3 индекса */
        m_indices.reserve(16 * 16 * 6);

        /* --- 1. Heightmap и Вершины --- */
        for (int z = 0; z < GRID_SIZE; z++) {
            for (int x = 0; x < GRID_SIZE; x++) {
                int idx = z * GRID_SIZE + x;

                float worldX = m_cx * 16 + x;
                float worldZ = m_cz * 16 + z;

                /* Получаем биом и высоту */
                auto sample = biomeNoise.sample(worldX, worldZ);
                float h = sample.height * TERRAIN_HEIGHT;
                m_heightmap[idx] = h;

                /* Позиция */
                m_vertices[idx].x = worldX;
                m_vertices[idx].y = h;
                m_vertices[idx].z = worldZ;

                /* Нормаль — approximate из соседей */
                float hL = (x > 0) ? m_heightmap[z * GRID_SIZE + (x - 1)] : h;
                float hR = (x < 16) ? m_heightmap[z * GRID_SIZE + (x + 1)] : h;
                float hD = (z > 0) ? m_heightmap[(z - 1) * GRID_SIZE + x] : h;
                float hU = (z < 16) ? m_heightmap[(z + 1) * GRID_SIZE + x] : h;

                float nx = hL - hR;
                float ny = 2.0f; // Чем больше число, тем более "размытыми/мягкими" кажутся склоны
                float nz = hD - hU;
                float len = std::sqrt(nx * nx + ny * ny + nz * nz);

                // Защита от деления на 0
                if (len > 0.0f) len = 1.0f / len;
                else len = 0.0f;

                m_vertices[idx].nx = nx * len;
                m_vertices[idx].ny = ny * len;
                m_vertices[idx].nz = nz * len;

                /* Цвет */
                auto params = biome::Biome::getParams(sample.biomeType);
                biome::BiomeColor blendedColor = params.color;

                // Упрощенный и более точный бленд (не больше 4 проверок)
                float blendRadius = 10.0f;
                float totalR = blendedColor.r, totalG = blendedColor.g, totalB = blendedColor.b;
                int neighborCount = 0;

                float offsets[4][2] = { {blendRadius, 0}, {-blendRadius, 0}, {0, blendRadius}, {0, -blendRadius} };
                for (int i = 0; i < 4; i++) {
                    auto neighborSample = biomeNoise.sample(worldX + offsets[i][0], worldZ + offsets[i][1]);
                    if (neighborSample.biomeType != sample.biomeType) {
                        auto nc = biome::Biome::getParams(neighborSample.biomeType).color;
                        totalR += nc.r; totalG += nc.g; totalB += nc.b;
                        neighborCount++;
                    }
                }

                if (neighborCount > 0) {
                    float weight = 1.0f / (neighborCount + 1);
                    totalR *= weight; totalG *= weight; totalB *= weight;
                }

                // Снежные вершины
                if (h > 100.0f) {
                    float snowFactor = std::min(1.0f, (h - 100.0f) / 100.0f);
                    m_vertices[idx].r = totalR * (1.0f - snowFactor) + 0.95f * snowFactor;
                    m_vertices[idx].g = totalG * (1.0f - snowFactor) + 0.95f * snowFactor;
                    m_vertices[idx].b = totalB * (1.0f - snowFactor) + 0.98f * snowFactor;
                }
                else {
                    m_vertices[idx].r = totalR;
                    m_vertices[idx].g = totalG;
                    m_vertices[idx].b = totalB;
                }
            }
        }

        /* --- 2. Индексы (Строго правильный CCW winding order) --- */
        for (int z = 0; z < 16; z++) {
            for (int x = 0; x < 16; x++) {
                uint32_t a = z * GRID_SIZE + x;
                uint32_t b = a + 1;
                uint32_t c = (z + 1) * GRID_SIZE + x;
                uint32_t d = c + 1;

                /*
                 * ФИКС: Правильный порядок против часовой стрелки (CCW),
                 * чтобы нормали смотрели наружу (вверх), а не в землю.
                 */
                m_indices.push_back(c); m_indices.push_back(b); m_indices.push_back(a); // Первый треугольник
                m_indices.push_back(c); m_indices.push_back(d); m_indices.push_back(b); // Второй треугольник
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

} // namespace be::void_::core::render::world::chunk