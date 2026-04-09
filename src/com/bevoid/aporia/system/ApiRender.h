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
 *
 * Десктоп: GLFW + OpenGL
 * Android: NativeActivity + EGL + GLES3
 */

#ifndef API_RENDER_H
#define API_RENDER_H

#include "system/OsManager.h"
#include <memory>

struct android_app;  /* Forward for Android stub */
struct GLFWwindow;

namespace com::bevoid::aporia::system {

/* ============================================================
 * Desktop: GLFW + OpenGL
 * ============================================================ */
#if !defined(BEVOID_PLATFORM_ANDROID)

class ApiRender {
public:
    ApiRender();
    ~ApiRender();

    bool create(const char* title, int width, int height);
    void shutdown();

    GLFWwindow*  getWindow() const { return m_window; }
    const OsManager& getOsManager() const { return m_osManager; }

    bool   shouldClose() const;
    void   swapBuffers();
    void   pollEvents();
    int32_t getWidth() const;
    int32_t getHeight() const;

    void setRenderCallback(void (*callback)(void* userData), void* userData) {
        m_renderCallback = callback;
        m_renderUserData  = userData;
    }
    void callRenderCallback() {
        if (m_renderCallback) m_renderCallback(m_renderUserData);
    }

    /* Stub on desktop — only used on Android */
    void registerAppCallbacks(struct android_app*) {}

private:
    OsManager   m_osManager;
    GLFWwindow* m_window = nullptr;
    void (*m_renderCallback)(void* userData) = nullptr;
    void*     m_renderUserData             = nullptr;

    /* Пустая структура — unique_ptr требует полный тип */
    struct AndroidState {};
    std::unique_ptr<AndroidState> m_androidState;
};

/* ============================================================
 * Android: NativeActivity + EGL + GLES3
 * ============================================================ */
#else /* BEVOID_PLATFORM_ANDROID */

class ApiRender {
public:
    ApiRender();
    ~ApiRender();

    bool create(const char* title, int width, int height);
    void shutdown();

    const OsManager& getOsManager() const { return m_osManager; }

    bool   shouldClose() const;
    void   swapBuffers();
    void   pollEvents();
    int32_t getWidth() const;
    int32_t getHeight() const;

    /* Android accessors */
    void* getAndroidActivity() const;
    void* getAndroidWindow() const;
    void* getEGLDisplay() const;
    void* getEGLSurface() const;
    void* getEGLContext() const;
    void* getEGLConfig() const;
    void  setEGLSurface(void* surface);

    /* Register APP_CMD callbacks — call from android_main */
    void  registerAppCallbacks(struct android_app* app);

    void setRenderCallback(void (*callback)(void* userData), void* userData) {
        m_renderCallback = callback;
        m_renderUserData  = userData;
    }
    void callRenderCallback() {
        if (m_renderCallback) m_renderCallback(m_renderUserData);
    }

    /* Static callbacks из Game.cpp имеют доступ */
    struct AndroidState;
    std::unique_ptr<AndroidState> m_androidState;

private:
    OsManager m_osManager;
    void (*m_renderCallback)(void* userData) = nullptr;
    void*     m_renderUserData             = nullptr;
};

#endif /* BEVOID_PLATFORM_ANDROID */

} // namespace com::bevoid::aporia::system

#endif // API_RENDER_H
