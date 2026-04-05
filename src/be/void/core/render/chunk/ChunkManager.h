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
 * be.void.core.render.chunk — ChunkManager
 *
 * Управляет чанками вокруг игрока.
 * - Фоновая генерация в std::thread
 * - Загрузка/выгрузка по расстоянию
 * - Мгновенная (мгновенная загрузка ближайших, фон — дальних)
 */

#ifndef BEVOID_CHUNK_MANAGER_H
#define BEVOID_CHUNK_MANAGER_H

#include "core/render/chunk/Chunk.h"
#include "core/render/chunk/Noise.h"
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <memory>

/* Те же константы что и в Chunk.cpp */
static constexpr float CM_TERRAIN_HEIGHT = 12.0f;
static constexpr float CM_TERRAIN_SCALE  = 0.04f;

namespace be::void_::core::render::chunk {

class ChunkManager {
public:
    ChunkManager();
    ~ChunkManager();

    /* Обновить — вызвать каждый кадр из главного потока */
    void update(float playerX, float playerZ, float deltaTime);

    /* Отрисовать все загруженные чанки */
    void draw() const;

    /* Настройки */
    void setRenderDistance(int chunks) { m_renderDist = chunks; }
    int  renderDistance() const { return m_renderDist; }

    /* Доступ к noise (для шейдеров и коллизий) */
    const Noise& getNoise() const { return m_noise; }

    /* Получить высоту террейна в точке мира */
    float getTerrainHeight(float worldX, float worldZ) const;

private:
    /* Ключ чанка */
    struct ChunkKey {
        int x, z;
        bool operator==(const ChunkKey& o) const { return x==o.x && z==o.z; }
    };

    struct ChunkKeyHash {
        std::size_t operator()(ChunkKey k) const {
            return std::hash<int>()(k.x) ^ (std::hash<int>()(k.z) << 16);
        }
    };

    /* Загрузить чанк (генерация в фоне) */
    void requestChunk(int cx, int cz);

    /* Выгрузить далёкие чанки */
    void unloadFarChunks(int playerCX, int playerCZ);

    /* Фоновый поток генерации */
    void workerThread();

    uint32_t    m_seed = 42;
    int         m_renderDist = 4;   /* радиус чанков */
    Noise       m_noise;

    /* Загруженные чанки (GL поток) */
    std::unordered_map<ChunkKey, std::unique_ptr<Chunk>, ChunkKeyHash> m_chunks;

    /* Очередь на генерацию (фон поток) */
    struct PendingChunk {
        int cx, cz;
        std::unique_ptr<Chunk> chunk;
    };

    std::vector<PendingChunk> m_pending;
    std::mutex                m_queueMutex;

    /* Фоновый поток */
    std::thread               m_worker;
    std::atomic<bool>         m_running = {true};
    std::atomic<int>          m_workerIndex = {-1};

    int m_lastPlayerCX = 0x7FFFFFFF;
    int m_lastPlayerCZ = 0x7FFFFFFF;
    float m_accumTime = 0;
};

} // namespace be::void_::core::render::chunk

#endif // BEVOID_CHUNK_MANAGER_H
