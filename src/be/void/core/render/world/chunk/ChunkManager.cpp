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
 * be.void.core.render.world.chunk — ChunkManager implementation
 *
 * Фоновая генерация чанков, загрузка GL, выгрузка далёких.
 */

#include "ChunkManager.h"
#include "../biome/BiomeNoise.h"
#include <algorithm>
#include <cmath>
#include <thread>
#include <chrono>
#include <random>

namespace be::void_::core::render::world::chunk {

ChunkManager::ChunkManager()
    : m_seed(std::random_device{}())
    , m_noise(m_seed)
    , m_biomeNoise(std::make_unique<biome::BiomeNoise>(m_seed))
{
    m_worker = std::thread(&ChunkManager::workerThread, this);
}

ChunkManager::~ChunkManager() {
    m_running = false;
    if (m_worker.joinable()) m_worker.join();
}

float ChunkManager::getTerrainHeight(float worldX, float worldZ) const {
    auto sample = m_biomeNoise->sample(worldX, worldZ);
    return sample.height * 200.0f;  // TERRAIN_HEIGHT = 200м
}

void ChunkManager::update(float playerX, float playerZ, float deltaTime) {
    int playerCX = static_cast<int>(std::floor(playerX / CHUNK_SIZE));
    int playerCZ = static_cast<int>(std::floor(playerZ / CHUNK_SIZE));

    m_accumTime += deltaTime;

    /* Обновляем чанки каждые 0.5 сек (не каждый кадр — экономия) */
    if (m_accumTime < 0.5f) return;
    m_accumTime = 0;

    /* Проверяем, сместился ли игрок */
    if (playerCX == m_lastPlayerCX && playerCZ == m_lastPlayerCZ) {
        /* Проверяем готовые чанки из фонового потока */
        std::lock_guard<std::mutex> lock(m_queueMutex);
        for (auto& pc : m_pending) {
            if (pc.chunk && pc.chunk->isGenerated() && !pc.chunk->isLoaded()) {
                pc.chunk->loadGL();
                ChunkKey key = {pc.cx, pc.cz};
                m_chunks[key] = std::move(pc.chunk);
            }
        }
        /* Убираем загруженные из pending */
        m_pending.erase(
            std::remove_if(m_pending.begin(), m_pending.end(),
                [](const PendingChunk& pc) {
                    return pc.chunk && pc.chunk->isLoaded();
                }),
            m_pending.end());
        return;
    }

    m_lastPlayerCX = playerCX;
    m_lastPlayerCZ = playerCZ;

    /* --- Выгрузить далёкие --- */
    unloadFarChunks(playerCX, playerCZ);

    /* --- Запросить новые --- */
    for (int dz = -m_renderDist; dz <= m_renderDist; dz++) {
        for (int dx = -m_renderDist; dx <= m_renderDist; dx++) {
            int cx = playerCX + dx;
            int cz = playerCZ + dz;
            ChunkKey key = {cx, cz};

            /* Уже есть или в очереди? */
            if (m_chunks.count(key)) continue;
            bool inQueue = false;
            {
                std::lock_guard<std::mutex> lock(m_queueMutex);
                for (auto& pc : m_pending) {
                    if (pc.cx == cx && pc.cz == cz) { inQueue = true; break; }
                }
            }
            if (inQueue) continue;

            /* Загружаем ближайший сразу, дальний — в фон */
            int dist = std::abs(dx) + std::abs(dz);
            if (dist <= 1) {
                /* Ближайший — синхронно (мгновенно) */
                auto chunk = std::make_unique<Chunk>(cx, cz, m_seed);
                chunk->generate(m_noise, *m_biomeNoise);
                chunk->loadGL();
                m_chunks[key] = std::move(chunk);
            } else {
                requestChunk(cx, cz);
            }
        }
    }

    /* Загрузить готовые из pending */
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        for (auto& pc : m_pending) {
            if (pc.chunk && pc.chunk->isGenerated() && !pc.chunk->isLoaded()) {
                pc.chunk->loadGL();
                ChunkKey key = {pc.cx, pc.cz};
                m_chunks[key] = std::move(pc.chunk);
            }
        }
        m_pending.erase(
            std::remove_if(m_pending.begin(), m_pending.end(),
                [](const PendingChunk& pc) {
                    return pc.chunk && pc.chunk->isLoaded();
                }),
            m_pending.end());
    }
}

void ChunkManager::draw() const {
    for (auto& [key, chunk] : m_chunks) {
        if (chunk && chunk->isLoaded()) chunk->draw();
    }
}

void ChunkManager::requestChunk(int cx, int cz) {
    std::lock_guard<std::mutex> lock(m_queueMutex);
    auto chunk = std::make_unique<Chunk>(cx, cz, m_seed);
    m_pending.push_back({cx, cz, std::move(chunk)});
}

void ChunkManager::unloadFarChunks(int playerCX, int playerCZ) {
    for (auto it = m_chunks.begin(); it != m_chunks.end(); ) {
        int dx = it->first.x - playerCX;
        int dz = it->first.z - playerCZ;
        int dist = std::abs(dx) + std::abs(dz);

        if (dist > m_renderDist + 1) {
            it->second->unloadGL();
            it = m_chunks.erase(it);
        } else {
            ++it;
        }
    }
}

void ChunkManager::workerThread() {
    while (m_running) {
        bool didWork = false;

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            for (auto& pc : m_pending) {
                if (pc.chunk && !pc.chunk->isGenerated() && !pc.chunk->isGenerating()) {
                    pc.chunk->generate(m_noise, *m_biomeNoise);
                    didWork = true;
                    break; /* один за итерацию — не блокируем надолго */
                }
            }
        }

        if (!didWork) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

} // namespace be::void_::core::render::world::chunk
