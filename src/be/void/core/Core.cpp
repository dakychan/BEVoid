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
 * be.void.core
 *
 * Реализация ядра — собирает все подсистемы.
 */

#include "core/Core.h"
#include <iostream>

namespace be::void_::core {

Core::Core() = default;
Core::~Core() = default;

bool Core::init() {
    std::cout << "[Core] Initializing subsystems...\n";

    m_input.setListener(&m_movement);

    if (!m_render.initShaders()) {
        std::cerr << "[Core] Failed to init shaders\n";
        return false;
    }
    if (!m_render.initChunks()) {
        std::cerr << "[Core] Failed to init chunks\n";
        return false;
    }

    auto& state = m_movement.getState();
    std::cout << "[Core] Player spawn at (" << state.position.x << ", " << state.position.y << ", " << state.position.z << ")\n";

    std::cout << "[Core] All subsystems initialized\n";
    return true;
}

void Core::shutdown() {
    m_render.shutdown();
    std::cout << "[Core] Shutdown complete\n";
}

void Core::update(float deltaTime) {
    auto camPos = m_movement.getCameraPos();
    float terrainH = m_render.getChunkManager().getTerrainHeight(camPos.x, camPos.z);

    m_movement.update(deltaTime, m_physics, terrainH);
    m_network.update(deltaTime);

    m_render.updateChunks(camPos.x, camPos.z, deltaTime);
}

void Core::render(float time) {
    auto& mv = m_movement;
    int w = 1280, h = 720;
    m_render.draw(time, mv.getCameraPos(), mv.getYaw(), mv.getPitch(), w, h);
}

} // namespace be::void_::core
