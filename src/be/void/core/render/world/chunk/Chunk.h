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
 * be.void.core.render.world.chunk — Chunk
 *
 * 64x64 блок террейна. Генерирует mesh из heightmap с биомами.
 * Генерация происходит асинхронно в фоновом потоке.
 */

#ifndef BE_VOID_CORE_RENDER_WORLD_CHUNK_H
#define BE_VOID_CORE_RENDER_WORLD_CHUNK_H

#include "Noise.h"
#include <cstdint>
#include <vector>
#include <atomic>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render::world::biome {
    class BiomeNoise;
}

namespace be::void_::core::render::world::chunk {

static constexpr int CHUNK_SIZE  = 64;   /* 64x64 блоков в чанке */
static constexpr int CHUNK_SCALE = 1;    /* 1 блок = 1 метр */
static constexpr int HEIGHT_MAP  = CHUNK_SIZE + 1;

struct ChunkVertex {
    float x, y, z;     /* позиция */
    float nx, ny, nz;  /* нормаль */
    float r, g, b;     /* цвет */
};

class Chunk {
public:
    Chunk(int cx, int cz, uint32_t seed);
    ~Chunk();

    /* Координаты чанка в мире */
    int chunkX() const { return m_cx; }
    int chunkZ() const { return m_cz; }

    /* Статус генерации */
    bool isGenerated() const  { return m_generated; }
    bool isLoaded()    const  { return m_glLoaded;  }
    bool isGenerating() const { return m_generating; }

    /* Запустить генерацию меша (вызывается из фонового потока) */
    void generate(const Noise& noise, const biome::BiomeNoise& biomeNoise);

    /* Загрузить GL буферы (вызывается из GL потока) */
    void loadGL();

    /* Отрисовать чанк */
    void draw() const;

    /* Удалить GL буферы */
    void unloadGL();

    /* Получить вершины (для отладки) */
    int vertexCount() const { return static_cast<int>(m_vertices.size()); }

private:
    int m_cx, m_cz;
    uint32_t m_seed;

    /* Heightmap */
    float m_heightmap[HEIGHT_MAP * HEIGHT_MAP] = {};

    /* Mesh данные */
    std::vector<ChunkVertex> m_vertices;
    std::vector<uint32_t>    m_indices;

    /* GL объекты */
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    /* Состояние */
    std::atomic<bool> m_generating = {false};
    bool m_generated = false;
    bool m_glLoaded = false;
};

} // namespace be::void_::core::render::world::chunk

#endif // BE_VOID_CORE_RENDER_WORLD_CHUNK_H
