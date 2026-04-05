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
#include <iostream>

/* --- Desktop: GLFW + OpenGL --- */
#if !defined(BEVOID_PLATFORM_ANDROID)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

namespace com::bevoid::aporia::system {

ApiRender::ApiRender() = default;
ApiRender::~ApiRender() = default;

bool ApiRender::create(const char* title, int width, int height) {
    m_osManager.detect();

    if (!glfwInit()) {
        std::cerr << "[ApiRender] glfwInit failed\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[ApiRender] glfwCreateWindow failed\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    /* Callbacks для drag/resize */
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

    glfwSetWindowUserPointer(m_window, this);

    std::cout << "[ApiRender] Window created: " << width << "x" << height << "\n";
    return true;
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

/* ============================================================
 * Android: EGL + OpenGL ES 3
 * ============================================================ */
#else /* BEVOID_PLATFORM_ANDROID */

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/native_activity.h>
#include <android/native_app_glue.h>
#include <android/log.h>
#include <android/input.h>
#include <android/looper.h>

#define LOG_TAG "ApiRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace com::bevoid::aporia::system {

/* Глобальный указатель для статических callbacks */
static ApiRender* g_apiRender = nullptr;

struct ApiRender::AndroidState {
    ANativeActivity* activity = nullptr;
    android_app*     app      = nullptr;
    ANativeWindow*   window   = nullptr;
    EGLDisplay       eglDisplay = EGL_NO_DISPLAY;
    EGLSurface       eglSurface = EGL_NO_SURFACE;
    EGLContext       eglContext = EGL_NO_CONTEXT;
    EGLConfig        eglConfig  = 0;
    int32_t          width = 0;
    int32_t          height = 0;
    bool             running = false;
    bool             hasFocus = false;
    bool             hasWindow = false;
};

ApiRender::ApiRender() : m_androidState(std::make_unique<AndroidState>()) {
    g_apiRender = this;
}

ApiRender::~ApiRender() = default;

static void handleCmd(android_app* app, int32_t cmd) {
    auto* state = g_apiRender->m_androidState.get();
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            state->window = app->window;
            if (state->window) {
                state->width  = ANativeWindow_getWidth(state->window);
                state->height = ANativeWindow_getHeight(state->window);
                state->hasWindow = true;
                LOGI("Window initialized: %dx%d", state->width, state->height);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            if (state->eglSurface != EGL_NO_SURFACE) {
                eglMakeCurrent(state->eglDisplay, EGL_NO_SURFACE,
                               EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroySurface(state->eglDisplay, state->eglSurface);
                state->eglSurface = EGL_NO_SURFACE;
            }
            state->window = nullptr;
            state->hasWindow = false;
            LOGI("Window terminated");
            break;
        case APP_CMD_GAINED_FOCUS:
            state->hasFocus = true;
            break;
        case APP_CMD_LOST_FOCUS:
            state->hasFocus = false;
            break;
    }
}

static int32_t handleInput(android_app* /*app*/, AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_KEY) {
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        if (keyCode == AKEYCODE_BACK) {
            g_apiRender->m_androidState->running = false;
            return 1;
        }
    }
    return 0;
}

bool ApiRender::create(const char* /*title*/, int /*width*/, int /*height*/) {
    m_osManager.detect();
    LOGI("ApiRender::create — Android EGL + GLES3");

    auto* state = m_androidState.get();
    /* android_app будет получен из android_main */

    /* EGL Display */
    state->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (state->eglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed: 0x%x", eglGetError());
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(state->eglDisplay, &major, &minor)) {
        LOGE("eglInitialize failed: 0x%x", eglGetError());
        return false;
    }
    LOGI("EGL %d.%d initialized", major, minor);

    /* EGL Config */
    const EGLint configAttrs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(state->eglDisplay, configAttrs,
                          &state->eglConfig, 1, &numConfigs)) {
        LOGE("eglChooseConfig failed: 0x%x", eglGetError());
        return false;
    }
    LOGI("EGL config chosen, configs available: %d", numConfigs);

    /* EGL Context */
    const EGLint ctxAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    state->eglContext = eglCreateContext(state->eglDisplay, state->eglConfig,
                                          EGL_NO_CONTEXT, ctxAttrs);
    if (state->eglContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed: 0x%x", eglGetError());
        return false;
    }
    LOGI("EGL context created");

    return true;
}

void ApiRender::shutdown() {
    auto* state = m_androidState.get();
    if (state->eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(state->eglDisplay, EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (state->eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(state->eglDisplay, state->eglSurface);
        if (state->eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(state->eglDisplay, state->eglContext);
        eglTerminate(state->eglDisplay);
        state->eglDisplay = EGL_NO_DISPLAY;
    }
    LOGI("ApiRender::shutdown");
}

bool ApiRender::shouldClose() const {
    return !m_androidState->running;
}

void ApiRender::swapBuffers() {
    auto* state = m_androidState.get();
    if (state->eglDisplay != EGL_NO_DISPLAY &&
        state->eglSurface != EGL_NO_SURFACE) {
        eglSwapBuffers(state->eglDisplay, state->eglSurface);
    }
}

void ApiRender::pollEvents() {
    /* На Android события обрабатываются в android_main */
}

int32_t ApiRender::getWidth() const {
    return m_androidState->width;
}

int32_t ApiRender::getHeight() const {
    return m_androidState->height;
}

/* Android entry point — вызывается из native_app_glue */
void android_main(struct android_app* app) {
    auto* state = g_apiRender->m_androidState.get();
    state->activity = app->activity;
    state->app = app;
    state->running = true;

    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    /* Главный цикл */
    while (state->running) {
        int events;
        struct android_poll_source* source;

        /* 0 = не блокировать при рендеринге */
        while (ALooper_pollAll(state->hasWindow ? 0 : -1, nullptr,
                                &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        /* Создаём surface если окно появилось */
        if (state->hasWindow && state->window &&
            state->eglSurface == EGL_NO_SURFACE) {
            state->eglSurface = eglCreateWindowSurface(
                state->eglDisplay, state->eglConfig, state->window, nullptr);
            if (state->eglSurface == EGL_NO_SURFACE) {
                LOGE("eglCreateWindowSurface failed: 0x%x", eglGetError());
            } else {
                if (!eglMakeCurrent(state->eglDisplay, state->eglSurface,
                                    state->eglSurface, state->eglContext)) {
                    LOGE("eglMakeCurrent failed: 0x%x", eglGetError());
                } else {
                    state->width  = ANativeWindow_getWidth(state->window);
                    state->height = ANativeWindow_getHeight(state->window);
                    glViewport(0, 0, state->width, state->height);
                    LOGI("EGL surface created: %dx%d",
                         state->width, state->height);
                }
            }
        }

        /* Рендер */
        if (state->eglSurface != EGL_NO_SURFACE && state->hasFocus) {
            g_apiRender->callRenderCallback();
            eglSwapBuffers(state->eglDisplay, state->eglSurface);
        }
    }
}

} // namespace com::bevoid::aporia::system

#endif /* BEVOID_PLATFORM_ANDROID */
