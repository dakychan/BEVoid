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
 * Делегирует оконный драйвер (IWindowDriver), платформы абстрагированы.
 */

#ifndef API_RENDER_H
#define API_RENDER_H

#include "system/OsManager.h"
#include "drivers/IWindowDriver.h"
#include <memory>

struct android_app;
struct GLFWwindow;

namespace com::bevoid::aporia::system {

class ApiRender {
public:
    ApiRender();
    ~ApiRender();

    bool create(const char* title, int width, int height);
    void shutdown();

    const OsManager& getOsManager() const { return m_osManager; }
    OsManager&       getOsManager()       { return m_osManager; }

    bool    shouldClose() const;
    void    swapBuffers();
    void    pollEvents();
    int32_t getWidth()  const;
    int32_t getHeight() const;

    void toggleFullscreen();
    bool isFullscreen() const;

    void setRenderCallback(void (*callback)(void* userData), void* userData) {
        m_renderCallback = callback;
        m_renderUserData = userData;
    }
    void callRenderCallback() {
        if (m_renderCallback) m_renderCallback(m_renderUserData);
    }

#if !defined(BEVOID_PLATFORM_ANDROID)
    GLFWwindow* getWindow() const;
#endif

#if defined(BEVOID_PLATFORM_ANDROID)
    void registerAppCallbacks(android_app* app);
#endif

private:
    OsManager m_osManager;
    std::unique_ptr<drivers::IWindowDriver> m_driver;
    void (*m_renderCallback)(void*) = nullptr;
    void* m_renderUserData = nullptr;
};

} // namespace com::bevoid::aporia::system

#endif // API_RENDER_H
