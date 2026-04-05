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

    if (!m_render.initShaders()) {
        std::cerr << "[Core] Failed to init shaders\n";
        return false;
    }
    if (!m_render.initGeometry()) {
        std::cerr << "[Core] Failed to init geometry\n";
        return false;
    }

    std::cout << "[Core] All subsystems initialized\n";
    return true;
}

void Core::shutdown() {
    m_render.shutdown();
    std::cout << "[Core] Shutdown complete\n";
}

void Core::update(float deltaTime) {
    m_movement.update(deltaTime);
    m_network.update(deltaTime);
    m_physics.update(deltaTime);
}

void Core::render(float time) {
    m_render.draw(time);
}

} // namespace be::void_::core
