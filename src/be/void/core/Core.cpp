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

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <android/log.h>
    #define LOG_TAG "BEVoid"
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
    #include <iostream>
    #define LOGI(...) std::printf(__VA_ARGS__)
    #define LOGE(...) std::fprintf(stderr, __VA_ARGS__)
#endif

namespace be::void_::core {

Core::Core() = default;
Core::~Core() = default;

bool Core::init() {
    LOGI("[Core] Initializing subsystems...\n");

    m_input.setListener(&m_movement);

    LOGI("[Core] initShaders...\n");
    if (!m_render.initShaders()) {
        LOGE("[Core] Failed to init shaders\n");
        return false;
    }
    LOGI("[Core] initSky...\n");
    if (!m_render.initSky()) {
        LOGI("[Core] WARNING: Sky init failed, sky will be disabled\n");
    }
    LOGI("[Core] initChunks...\n");
    if (!m_render.initChunks()) {
        LOGE("[Core] Failed to init chunks\n");
        return false;
    }

    auto& state = m_movement.getState();
    LOGI("[Core] Player spawn at (%.1f, %.1f, %.1f)\n",
         state.position.x, state.position.y, state.position.z);
    LOGI("[Core] All subsystems initialized\n");
    return true;
}

void Core::shutdown() {
    m_render.shutdown();
    LOGI("[Core] Shutdown complete\n");
}

void Core::update(float deltaTime) {
    auto camPos = m_movement.getCameraPos();
    float terrainH = m_render.getChunkManager().terrainHeight(camPos.x, camPos.z);

    m_movement.update(deltaTime, m_physics, terrainH);
    m_network.update(deltaTime);

    m_render.updateChunks(camPos.x, camPos.z, deltaTime);
}

void Core::render(float time, int viewportW, int viewportH) {
    auto& mv = m_movement;
    Vec3 camPos = mv.getCameraPos();
    camPos.y -= mv.getCrouchOffset();
    m_render.draw(time, camPos, mv.getYaw(), mv.getPitch(), viewportW, viewportH);
}

} // namespace be::void_::core
