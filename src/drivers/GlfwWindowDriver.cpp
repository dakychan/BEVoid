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
 * BEVoid Project — drivers
 *
 * GLFW window driver — десктоп.
 */

#include "drivers/GlfwWindowDriver.h"

#if !defined(BEVOID_PLATFORM_ANDROID)

#if defined(BEVOID_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <iostream>

namespace drivers {

GlfwWindowDriver::~GlfwWindowDriver() { shutdown(); }

bool GlfwWindowDriver::init(const WindowConfig& config) {
    if (!glfwInit()) {
        std::cerr << "[GlfwWindowDriver] glfwInit failed\n";
        return false;
    }
    m_glfwInit = true;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, config.resizable ? GLFW_TRUE : GLFW_FALSE);

    m_windowedW = config.width;
    m_windowedH = config.height;

    if (config.fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        m_window = glfwCreateWindow(mode->width, mode->height,
                                     config.title.c_str(), monitor, nullptr);
        m_fullscreen = true;
    } else {
        m_window = glfwCreateWindow(config.width, config.height,
                                     config.title.c_str(), nullptr, nullptr);
    }

    if (!m_window) {
        std::cerr << "[GlfwWindowDriver] glfwCreateWindow failed\n";
        glfwTerminate();
        m_glfwInit = false;
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    std::cout << "[GlfwWindowDriver] Window created: "
              << config.width << "x" << config.height << "\n";
    return true;
}

void GlfwWindowDriver::shutdown() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    if (m_glfwInit) {
        glfwTerminate();
        m_glfwInit = false;
    }
}

bool GlfwWindowDriver::pollEvents() {
    glfwPollEvents();
    return !glfwWindowShouldClose(m_window);
}

bool GlfwWindowDriver::shouldClose() const {
    return m_window ? glfwWindowShouldClose(m_window) : true;
}

void GlfwWindowDriver::setTitle(const std::string& title) {
    if (m_window) glfwSetWindowTitle(m_window, title.c_str());
}

void GlfwWindowDriver::swapBuffers() {
    if (m_window) glfwSwapBuffers(m_window);
}

int32_t GlfwWindowDriver::getWidth() const {
    if (m_window) {
        int w; glfwGetWindowSize(m_window, &w, nullptr);
        return static_cast<int32_t>(w);
    }
    return 0;
}

int32_t GlfwWindowDriver::getHeight() const {
    if (m_window) {
        int h; glfwGetWindowSize(m_window, nullptr, &h);
        return static_cast<int32_t>(h);
    }
    return 0;
}

void* GlfwWindowDriver::getNativeWindow() const { return m_window; }

void GlfwWindowDriver::setFullscreen(bool fullscreen) {
    if (!m_window || m_fullscreen == fullscreen) return;

    if (fullscreen) {
        glfwGetWindowPos(m_window, &m_windowedX, &m_windowedY);
        glfwGetWindowSize(m_window, &m_windowedW, &m_windowedH);
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(m_window, monitor, 0, 0,
                             mode->width, mode->height, mode->refreshRate);
    } else {
        glfwSetWindowMonitor(m_window, nullptr,
                             m_windowedX, m_windowedY,
                             m_windowedW, m_windowedH, 0);
    }
    m_fullscreen = fullscreen;
}

} // namespace drivers

#endif // !BEVOID_PLATFORM_ANDROID
