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
 * BEVoid Project — com.bevoid.aporia.system
 *
 * ApiRender — фабрика рендера.
 * Для десктопа: GLFW + OpenGL. Просто и работает.
 */

#include "system/ApiRender.h"

/* GLFW без gl.h — glad вместо него */
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>

namespace com::bevoid::aporia::system {

ApiRender::ApiRender() = default;
ApiRender::~ApiRender() = default;

bool ApiRender::create(const char* title, int width, int height) {
    /* Определяем платформу */
    m_osManager.detect();

    /* --- GLFW для десктопа --- */
#if defined(BEVOID_PLATFORM_WINDOWS) || defined(BEVOID_PLATFORM_LINUX) || defined(BEVOID_PLATFORM_MACOS)

    if (!glfwInit()) {
        std::cerr << "[ApiRender] glfwInit failed\n";
        return false;
    }

    /* OpenGL 3.3 Core */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

#if defined(BEVOID_PLATFORM_ANDROID)
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
#endif

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[ApiRender] glfwCreateWindow failed\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // VSync

    /* --- Callback: перерисовка во время resize/drag (Win32 modal loop) --- */
    /* При drag/resize на Windows, pollEvents входит в modal loop.
       Refresh callback вызывается когда Windows просит перерисовку окна. */
    glfwSetWindowRefreshCallback(m_window, [](GLFWwindow* win) {
        ApiRender* self = reinterpret_cast<ApiRender*>(glfwGetWindowUserPointer(win));
        if (self) {
            self->callRenderCallback();
            glfwSwapBuffers(win);
        }
    });

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* win, int w, int h) {
        glViewport(0, 0, w, h);
        ApiRender* self = reinterpret_cast<ApiRender*>(glfwGetWindowUserPointer(win));
        if (self) {
            self->callRenderCallback();
            glfwSwapBuffers(win);
        }
    });

    /* Сохраняем указатель на ApiRender в окне для callbacks */
    glfwSetWindowUserPointer(m_window, this);

    std::cout << "[ApiRender] Window created: " << width << "x" << height << "\n";
    return true;

#else
    std::cerr << "[ApiRender] Platform not supported\n";
    return false;
#endif
}

void ApiRender::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
}

bool ApiRender::shouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void ApiRender::swapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

void ApiRender::pollEvents() {
    glfwPollEvents();
}

int32_t ApiRender::getWidth() const {
    if (m_window) {
        int w; glfwGetWindowSize(m_window, &w, nullptr);
        return static_cast<int32_t>(w);
    }
    return 0;
}

int32_t ApiRender::getHeight() const {
    if (m_window) {
        int h; glfwGetWindowSize(m_window, nullptr, &h);
        return static_cast<int32_t>(h);
    }
    return 0;
}

} // namespace com::bevoid::aporia::system
