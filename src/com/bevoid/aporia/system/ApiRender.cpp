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
 * Делегирует оконный драйвер (IWindowDriver).
 */

#include "system/ApiRender.h"
#include <iostream>

/* ---- Desktop: GLFW ---- */
#if !defined(BEVOID_PLATFORM_ANDROID)
#include "drivers/GlfwWindowDriver.h"

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
#include <glad/glad.h>

/* ---- Android: EGL ---- */
#else
#include "drivers/EglWindowDriver.h"
#endif

namespace com::bevoid::aporia::system {

ApiRender::ApiRender() = default;
ApiRender::~ApiRender() = default;

bool ApiRender::create(const char* title, int width, int height) {
    m_osManager.detect();

    drivers::WindowConfig config;
    config.title     = title;
    config.width     = width;
    config.height    = height;
    config.resizable = true;

#if !defined(BEVOID_PLATFORM_ANDROID)
    m_driver = std::make_unique<drivers::GlfwWindowDriver>();
#else
    m_driver = std::make_unique<drivers::EglWindowDriver>();
#endif

    if (!m_driver->init(config)) {
        std::cerr << "[ApiRender] Driver init failed\n";
        return false;
    }

    /* --- Desktop: GLFW callbacks + icon --- */
#if !defined(BEVOID_PLATFORM_ANDROID)
    auto* win = static_cast<GLFWwindow*>(m_driver->getNativeWindow());
    glfwSetWindowUserPointer(win, this);

#ifdef BEVOID_PLATFORM_WINDOWS
    {
        HICON hIcon = (HICON)LoadImage(
            GetModuleHandle(NULL), MAKEINTRESOURCE(101),
            IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
        if (hIcon) {
            HDC hdc    = GetDC(NULL);
            HDC hdcMem = CreateCompatibleDC(hdc);

            BITMAPINFO bi  = {};
            bi.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth  = 32;
            bi.bmiHeader.biHeight = -32;
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;

            unsigned char* pixels = nullptr;
            HBITMAP hbmp = CreateDIBSection(hdcMem, &bi,
                                             DIB_RGB_COLORS,
                                             (void**)&pixels, NULL, 0);
            if (hbmp && pixels) {
                HGDIOBJ hOld = SelectObject(hdcMem, hbmp);
                DrawIconEx(hdcMem, 0, 0, hIcon, 32, 32, 0, NULL, DI_NORMAL);

                unsigned char* iconData = new unsigned char[32 * 32 * 4];
                memcpy(iconData, pixels, 32 * 32 * 4);
                for (int i = 0; i < 32 * 32; i++) {
                    unsigned char t = iconData[i * 4];
                    iconData[i * 4]     = iconData[i * 4 + 2];
                    iconData[i * 4 + 2] = t;
                }

                GLFWimage icon;
                icon.width  = 32;
                icon.height = 32;
                icon.pixels = iconData;
                glfwSetWindowIcon(win, 1, &icon);

                SelectObject(hdcMem, hOld);
                DeleteObject(hbmp);
                delete[] iconData;
            }
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdc);
            DestroyIcon(hIcon);
        }
    }
#endif

    glfwSetWindowRefreshCallback(win, [](GLFWwindow* w) {
        auto* self = static_cast<ApiRender*>(glfwGetWindowUserPointer(w));
        if (self) {
            self->callRenderCallback();
            glfwSwapBuffers(w);
        }
    });

    glfwSetWindowSizeCallback(win, [](GLFWwindow* w, int ww, int hh) {
        glViewport(0, 0, ww, hh);
        auto* self = static_cast<ApiRender*>(glfwGetWindowUserPointer(w));
        if (self) {
            self->callRenderCallback();
            glfwSwapBuffers(w);
        }
    });
#endif

    std::cout << "[ApiRender] Window created: " << width << "x" << height << "\n";
    return true;
}

void ApiRender::shutdown() {
    if (m_driver) {
        m_driver->shutdown();
        m_driver.reset();
    }
}

bool ApiRender::shouldClose() const {
    return m_driver ? m_driver->shouldClose() : true;
}

void ApiRender::swapBuffers() {
    if (m_driver) m_driver->swapBuffers();
}

void ApiRender::pollEvents() {
    if (m_driver) m_driver->pollEvents();
}

int32_t ApiRender::getWidth() const {
    return m_driver ? m_driver->getWidth() : 0;
}

int32_t ApiRender::getHeight() const {
    return m_driver ? m_driver->getHeight() : 0;
}

void ApiRender::toggleFullscreen() {
    if (m_driver) m_driver->setFullscreen(!m_driver->isFullscreen());
}

bool ApiRender::isFullscreen() const {
    return m_driver ? m_driver->isFullscreen() : false;
}

/* --- Desktop-only --- */
#if !defined(BEVOID_PLATFORM_ANDROID)
GLFWwindow* ApiRender::getWindow() const {
    return m_driver
        ? static_cast<GLFWwindow*>(m_driver->getNativeWindow())
        : nullptr;
}
#endif

/* --- Android-only --- */
#if defined(BEVOID_PLATFORM_ANDROID)
void ApiRender::registerAppCallbacks(android_app* app) {
    auto* egl = static_cast<drivers::EglWindowDriver*>(m_driver.get());
    if (egl) egl->registerAppCallbacks(app);
}
#endif

} // namespace com::bevoid::aporia::system
