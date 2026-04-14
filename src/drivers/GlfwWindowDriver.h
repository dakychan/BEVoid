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
 * GLFW window driver — десктоп (Windows / Linux / macOS).
 */

#ifndef GLFW_WINDOW_DRIVER_H
#define GLFW_WINDOW_DRIVER_H

#include "IWindowDriver.h"

struct GLFWwindow;

namespace drivers {

class GlfwWindowDriver : public IWindowDriver {
public:
    GlfwWindowDriver() = default;
    ~GlfwWindowDriver() override;

    bool   init(const WindowConfig& config) override;
    void   shutdown() override;
    bool   pollEvents() override;
    bool   shouldClose() const override;
    void   setTitle(const std::string& title) override;
    void   swapBuffers() override;
    int32_t getWidth() const override;
    int32_t getHeight() const override;
    void*  getNativeWindow() const override;
    void   setFullscreen(bool fullscreen) override;
    bool   isFullscreen() const override { return m_fullscreen; }

    GLFWwindow* getGlfwWindow() const { return m_window; }

private:
    GLFWwindow* m_window = nullptr;
    bool m_fullscreen = false;
    bool m_glfwInit = false;
    int  m_windowedX = 0, m_windowedY = 0;
    int  m_windowedW = 1280, m_windowedH = 720;
};

} // namespace drivers

#endif // GLFW_WINDOW_DRIVER_H
