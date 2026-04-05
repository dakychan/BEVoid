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
 * Резолвит через OsManager платформу и создаёт нужные драйвер+рендерер.
 *
 * Десктоп: GLFW + OpenGL
 * Android: NativeActivity + EGL (TODO)
 */

#ifndef API_RENDER_H
#define API_RENDER_H

#include "system/OsManager.h"
#include <memory>

struct GLFWwindow;

namespace com::bevoid::aporia::system {

class ApiRender {
public:
    ApiRender();
    ~ApiRender();

    /* Создать окно + OpenGL контекст для текущей платформы */
    bool create(const char* title, int width, int height);

    /* Закрыть всё */
    void shutdown();

    /* Доступ */
#if !defined(BEVOID_PLATFORM_ANDROID)
    GLFWwindow*  getWindow() const { return m_window; }
#endif
    const OsManager& getOsManager() const { return m_osManager; }

    /* Удобные методы */
    bool   shouldClose() const;
    void   swapBuffers();
    void   pollEvents();
    int32_t getWidth() const;
    int32_t getHeight() const;

    /* Android: доступ к состоянию из Game.cpp android_main */
#if defined(BEVOID_PLATFORM_ANDROID)
    void* getAndroidActivity() const;
    void* getAndroidWindow() const;
    void* getEGLDisplay() const;
    void* getEGLSurface() const;
    void* getEGLContext() const;
    void* getEGLConfig() const;
    void  setEGLSurface(void* surface);
    void  onAndroidWindowCreated();
    void  onAndroidWindowDestroyed();
#endif

    /* Callback для рендера */
    void setRenderCallback(void (*callback)(void* userData), void* userData) {
        m_renderCallback = callback;
        m_renderUserData  = userData;
    }
    void callRenderCallback() {
        if (m_renderCallback) m_renderCallback(m_renderUserData);
    }

    /* Android: доступ к состоянию из static callbacks */
    struct AndroidState;
    std::unique_ptr<AndroidState> m_androidState;

private:
    OsManager m_osManager;

#if !defined(BEVOID_PLATFORM_ANDROID)
    GLFWwindow* m_window = nullptr;
    /* Пустая структура чтобы unique_ptr имел полный тип */
    struct AndroidState {};
    std::unique_ptr<AndroidState> m_androidState;
#else
    struct AndroidState;
    std::unique_ptr<AndroidState> m_androidState;
#endif

    void (*m_renderCallback)(void* userData) = nullptr;
    void*     m_renderUserData             = nullptr;
};

} // namespace com::bevoid::aporia::system

#endif // API_RENDER_H
