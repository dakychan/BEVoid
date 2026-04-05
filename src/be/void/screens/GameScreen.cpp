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
 * be.void.screens — GameScreen implementation
 *
 * Создаёт ApiRender + Core, запускает игровой цикл.
 */

#include "screens/GameScreen.h"
#include <iostream>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#endif

namespace be::void_::screens {

void GameScreen::onEnter() {
    std::cout << "[GameScreen] Enter\n";
    m_api = std::make_unique<com::bevoid::aporia::system::ApiRender>();
    if (!m_api->create("BEVoid", 1280, 720)) {
        std::cerr << "[GameScreen] ApiRender::create failed\n";
        return;
    }

#if !defined(BEVOID_PLATFORM_ANDROID)
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
#endif

    if (!m_core.init()) {
        std::cerr << "[GameScreen] Core init failed\n";
        return;
    }

    glEnable(GL_DEPTH_TEST);
    m_initialized = true;
    m_time = 0;
}

void GameScreen::onExit() {
    std::cout << "[GameScreen] Exit\n";
    if (m_initialized) {
        m_core.shutdown();
        m_api.reset();
        m_initialized = false;
    }
}

void GameScreen::update(float dt) {
    if (!m_initialized) return;
    m_time += dt;
    m_core.update(dt);

    /* ESC — вернуться в меню */
    /* В реальном проекте — через InputManager */
}

void GameScreen::render(float /*time*/) {
    if (!m_initialized) return;

    auto pos = m_core.getMovement().getCameraPos();
    m_core.getRender().updateChunks(pos.x, pos.z, 0.016f);

    m_core.render(m_time);
    m_api->swapBuffers();
}

} // namespace be::void_::screens
