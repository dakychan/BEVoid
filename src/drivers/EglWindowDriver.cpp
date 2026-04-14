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
 * EGL window driver — Android.
 */

#include "drivers/EglWindowDriver.h"

#if defined(BEVOID_PLATFORM_ANDROID)

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#include <android/native_activity.h>
#include <android/log.h>
#include <android/input.h>
#include "android_native_app_glue.h"

#define LOG_TAG "EglWindowDriver"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace drivers {

static EglWindowDriver* g_driver = nullptr;

struct EglWindowDriver::AndroidState {
    android_app*   app       = nullptr;
    ANativeWindow* window    = nullptr;
    EGLDisplay     eglDisplay = EGL_NO_DISPLAY;
    EGLSurface     eglSurface = EGL_NO_SURFACE;
    EGLContext     eglContext = EGL_NO_CONTEXT;
    EGLConfig      eglConfig  = 0;
    int32_t        width  = 0;
    int32_t        height = 0;
    bool           hasWindow = false;
    bool           hasFocus  = false;
    bool           running   = true;
};

EglWindowDriver::EglWindowDriver()
    : m_state(std::make_unique<AndroidState>()) {
    g_driver = this;
}

EglWindowDriver::~EglWindowDriver() { shutdown(); }

void EglWindowDriver::handleCmd(android_app* app, int32_t cmd) {
    auto* s = g_driver->m_state.get();
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            s->window = app->window;
            if (s->window) {
                s->width  = ANativeWindow_getWidth(s->window);
                s->height = ANativeWindow_getHeight(s->window);
                s->hasWindow = true;
                LOGI("Window initialized: %dx%d", s->width, s->height);
                if (s->eglDisplay != EGL_NO_DISPLAY &&
                    s->eglContext != EGL_NO_CONTEXT) {
                    EGLint surfAttrs[] = { EGL_NONE };
                    s->eglSurface = eglCreateWindowSurface(
                        s->eglDisplay, s->eglConfig, s->window, surfAttrs);
                    if (s->eglSurface == EGL_NO_SURFACE) {
                        LOGE("eglCreateWindowSurface failed: 0x%x", eglGetError());
                    } else if (!eglMakeCurrent(s->eglDisplay, s->eglSurface,
                                               s->eglSurface, s->eglContext)) {
                        LOGE("eglMakeCurrent(window) failed: 0x%x", eglGetError());
                    } else {
                        LOGI("EGL surface created and context made current");
                    }
                }
            }
            break;

        case APP_CMD_TERM_WINDOW:
            if (s->eglSurface != EGL_NO_SURFACE) {
                eglMakeCurrent(s->eglDisplay, EGL_NO_SURFACE,
                               EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroySurface(s->eglDisplay, s->eglSurface);
                s->eglSurface = EGL_NO_SURFACE;
            }
            s->window = nullptr;
            s->hasWindow = false;
            LOGI("Window terminated");
            break;

        case APP_CMD_GAINED_FOCUS: s->hasFocus = true;  break;
        case APP_CMD_LOST_FOCUS:   s->hasFocus = false; break;

        case APP_CMD_WINDOW_RESIZED:
            if (s->window) {
                s->width  = ANativeWindow_getWidth(s->window);
                s->height = ANativeWindow_getHeight(s->window);
                glViewport(0, 0, s->width, s->height);
            }
            break;

        case APP_CMD_CONFIG_CHANGED:
            if (s->window) {
                s->width  = ANativeWindow_getWidth(s->window);
                s->height = ANativeWindow_getHeight(s->window);
            }
            break;
    }
}

bool EglWindowDriver::init(const WindowConfig& /*config*/) {
    auto* s = m_state.get();

    s->eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (s->eglDisplay == EGL_NO_DISPLAY) {
        LOGE("eglGetDisplay failed: 0x%x", eglGetError());
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(s->eglDisplay, &major, &minor)) {
        LOGE("eglInitialize failed: 0x%x", eglGetError());
        return false;
    }
    LOGI("EGL %d.%d initialized", major, minor);

    const EGLint configAttrs[] = {
        EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };

    EGLint numConfigs;
    if (!eglChooseConfig(s->eglDisplay, configAttrs,
                          &s->eglConfig, 1, &numConfigs)) {
        LOGE("eglChooseConfig failed: 0x%x", eglGetError());
        return false;
    }

    const EGLint ctxAttrs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 3,
        EGL_NONE
    };
    s->eglContext = eglCreateContext(s->eglDisplay, s->eglConfig,
                                      EGL_NO_CONTEXT, ctxAttrs);
    if (s->eglContext == EGL_NO_CONTEXT) {
        LOGE("eglCreateContext failed: 0x%x", eglGetError());
        return false;
    }

    const EGLint pbufAttrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLSurface pbuf = eglCreatePbufferSurface(
        s->eglDisplay, s->eglConfig, pbufAttrs);
    if (pbuf != EGL_NO_SURFACE) {
        if (!eglMakeCurrent(s->eglDisplay, pbuf, pbuf, s->eglContext)) {
            LOGE("eglMakeCurrent(pbuffer) failed: 0x%x", eglGetError());
        } else {
            LOGI("EGL context made current via pbuffer");
        }
    }

    return true;
}

void EglWindowDriver::shutdown() {
    auto* s = m_state.get();
    if (s->eglDisplay != EGL_NO_DISPLAY) {
        eglMakeCurrent(s->eglDisplay, EGL_NO_SURFACE,
                       EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s->eglSurface != EGL_NO_SURFACE)
            eglDestroySurface(s->eglDisplay, s->eglSurface);
        if (s->eglContext != EGL_NO_CONTEXT)
            eglDestroyContext(s->eglDisplay, s->eglContext);
        eglTerminate(s->eglDisplay);
        s->eglDisplay = EGL_NO_DISPLAY;
    }
}

bool EglWindowDriver::pollEvents() { return m_state->running; }

bool EglWindowDriver::shouldClose() const { return !m_state->running; }

void EglWindowDriver::setTitle(const std::string&) {}

void EglWindowDriver::swapBuffers() {
    auto* s = m_state.get();
    if (s->eglDisplay == EGL_NO_DISPLAY ||
        s->eglSurface == EGL_NO_SURFACE) return;

    if (!eglSwapBuffers(s->eglDisplay, s->eglSurface)) {
        EGLint error = eglGetError();
        if (error == EGL_BAD_SURFACE || error == EGL_BAD_NATIVE_WINDOW) {
            LOGW("eglSwapBuffers failed, recreating surface: 0x%x", error);
            eglMakeCurrent(s->eglDisplay, EGL_NO_SURFACE,
                           EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (s->eglSurface != EGL_NO_SURFACE) {
                eglDestroySurface(s->eglDisplay, s->eglSurface);
                s->eglSurface = EGL_NO_SURFACE;
            }
            if (s->window && s->eglContext != EGL_NO_CONTEXT) {
                EGLint surfAttrs[] = { EGL_NONE };
                s->eglSurface = eglCreateWindowSurface(
                    s->eglDisplay, s->eglConfig, s->window, surfAttrs);
                if (s->eglSurface != EGL_NO_SURFACE) {
                    eglMakeCurrent(s->eglDisplay, s->eglSurface,
                                   s->eglSurface, s->eglContext);
                    LOGI("EGL surface recreated");
                }
            }
        }
    }
}

int32_t EglWindowDriver::getWidth()  const { return m_state->width;  }
int32_t EglWindowDriver::getHeight() const { return m_state->height; }
void*   EglWindowDriver::getNativeWindow() const { return m_state->window; }

void EglWindowDriver::setFullscreen(bool) {}

void EglWindowDriver::registerAppCallbacks(android_app* app) {
    m_state->app = app;
    app->onAppCmd = handleCmd;
    LOGI("APP_CMD callbacks registered");
}

} // namespace drivers

#endif // BEVOID_PLATFORM_ANDROID
