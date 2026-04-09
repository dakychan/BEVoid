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

/* Windows headers MUST come before GLFW to avoid APIENTRY redefinition */
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

    glfwSetWindowUserPointer(m_window, this);

    // Set window icon from embedded resource (Windows)
    #ifdef BEVOID_PLATFORM_WINDOWS
    {
        // Load icon from .exe resource
        HICON hIcon = (HICON)LoadImage(
            GetModuleHandle(NULL),
            MAKEINTRESOURCE(101),  // IDI_ICON1 from .rc file
            IMAGE_ICON,
            32, 32,
            LR_DEFAULTCOLOR
        );
        
        if (hIcon) {
            // Convert HICON to GLFWimage format (32x32 RGBA)
            HDC hdc = GetDC(NULL);
            HDC hdcMem = CreateCompatibleDC(hdc);
            
            BITMAPINFO bi = {0};
            bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bi.bmiHeader.biWidth = 32;
            bi.bmiHeader.biHeight = -32;  // Top-down
            bi.bmiHeader.biPlanes = 1;
            bi.bmiHeader.biBitCount = 32;
            bi.bmiHeader.biCompression = BI_RGB;
            
            unsigned char* pixels = nullptr;
            HBITMAP hbmp = CreateDIBSection(hdcMem, &bi, DIB_RGB_COLORS, (void**)&pixels, NULL, 0);
            
            if (hbmp && pixels) {
                HGDIOBJ hOldBmp = SelectObject(hdcMem, hbmp);
                
                // Draw icon to bitmap
                DrawIconEx(hdcMem, 0, 0, hIcon, 32, 32, 0, NULL, DI_NORMAL);
                
                // Copy pixels before cleanup (GDI owns the memory after CreateDIBSection)
                unsigned char* iconData = new unsigned char[32 * 32 * 4];
                memcpy(iconData, pixels, 32 * 32 * 4);
                
                // Convert BGRA to RGBA
                for (int i = 0; i < 32 * 32; i++) {
                    unsigned char temp = iconData[i * 4];
                    iconData[i * 4] = iconData[i * 4 + 2];     // R <-> B
                    iconData[i * 4 + 2] = temp;
                }
                
                GLFWimage icon;
                icon.width = 32;
                icon.height = 32;
                icon.pixels = iconData;
                glfwSetWindowIcon(m_window, 1, &icon);
                
                // Cleanup GDI objects (GDI frees 'pixels' when hbmp is deleted)
                SelectObject(hdcMem, hOldBmp);
                DeleteObject(hbmp);
                // GLFW copies the icon data internally, so we can free ours
                delete[] iconData;
            }
            
            DeleteDC(hdcMem);
            ReleaseDC(NULL, hdc);
            DestroyIcon(hIcon);
        }
    }
    #endif

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
#include <android/log.h>
#include <android/input.h>
#include <android/looper.h>
#include "android_native_app_glue.h"

#define LOG_TAG "ApiRender"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace com::bevoid::aporia::system {

/* Глобальный указатель для статических callbacks */
static ApiRender* g_apiRender = nullptr;

/* Forward declaration */
static void handleCmd(android_app* app, int32_t cmd);

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

                /* --- Создаём EGL surface для окна --- */
                if (state->eglDisplay != EGL_NO_DISPLAY && state->eglContext != EGL_NO_CONTEXT) {
                    EGLint surfAttrs[] = { EGL_NONE };
                    state->eglSurface = eglCreateWindowSurface(
                        state->eglDisplay, state->eglConfig, state->window, surfAttrs);
                    
                    if (state->eglSurface == EGL_NO_SURFACE) {
                        LOGE("eglCreateWindowSurface failed: 0x%x", eglGetError());
                    } else {
                        /* Делаем контекст активным для реального окна */
                        if (!eglMakeCurrent(state->eglDisplay, state->eglSurface,
                                           state->eglSurface, state->eglContext)) {
                            LOGE("eglMakeCurrent(window) failed: 0x%x", eglGetError());
                        } else {
                            LOGI("EGL surface created and context made current");
                            /* Вызываем рендер callback сразу после инициализации */
                            g_apiRender->callRenderCallback();
                            eglSwapBuffers(state->eglDisplay, state->eglSurface);
                        }
                    }
                }
            }
            break;

        case APP_CMD_TERM_WINDOW:
            if (state->eglSurface != EGL_NO_SURFACE) {
                eglMakeCurrent(state->eglDisplay, EGL_NO_SURFACE,
                               EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroySurface(state->eglDisplay, state->eglSurface);
                state->eglSurface = EGL_NO_SURFACE;
                LOGI("EGL surface destroyed");
            }
            state->window = nullptr;
            state->hasWindow = false;
            LOGI("Window terminated");
            break;

        case APP_CMD_GAINED_FOCUS:
            state->hasFocus = true;
            LOGI("Focus gained");
            break;

        case APP_CMD_LOST_FOCUS:
            state->hasFocus = false;
            LOGI("Focus lost");
            break;

        case APP_CMD_WINDOW_RESIZED:
            if (state->window) {
                state->width  = ANativeWindow_getWidth(state->window);
                state->height = ANativeWindow_getHeight(state->window);
                LOGI("Window resized: %dx%d", state->width, state->height);
                glViewport(0, 0, state->width, state->height);
            }
            break;

        case APP_CMD_CONFIG_CHANGED:
            if (state->window) {
                state->width  = ANativeWindow_getWidth(state->window);
                state->height = ANativeWindow_getHeight(state->window);
                LOGI("Config changed: %dx%d", state->width, state->height);
            }
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

    /* --- Временный pbuffer — чтобы контекст стал активным --- */
    /* Без eglMakeCurrent GL вызовы (шейдеры, VAO) молча падают */
    const EGLint pbufAttrs[] = { EGL_WIDTH, 1, EGL_HEIGHT, 1, EGL_NONE };
    EGLSurface pbuf = eglCreatePbufferSurface(state->eglDisplay, state->eglConfig, pbufAttrs);
    if (pbuf != EGL_NO_SURFACE) {
        if (!eglMakeCurrent(state->eglDisplay, pbuf, pbuf, state->eglContext)) {
            LOGE("eglMakeCurrent(pbuffer) failed: 0x%x", eglGetError());
        } else {
            LOGI("EGL context made current via pbuffer — GL ready");
        }
    }

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
        if (!eglSwapBuffers(state->eglDisplay, state->eglSurface)) {
            EGLint error = eglGetError();
            if (error == EGL_BAD_SURFACE || error == EGL_BAD_NATIVE_WINDOW) {
                LOGW("eglSwapBuffers failed (surface invalid), recreating: 0x%x", error);
                /* Попробуем пересоздать surface */
                eglMakeCurrent(state->eglDisplay, EGL_NO_SURFACE,
                               EGL_NO_SURFACE, EGL_NO_CONTEXT);
                if (state->eglSurface != EGL_NO_SURFACE) {
                    eglDestroySurface(state->eglDisplay, state->eglSurface);
                    state->eglSurface = EGL_NO_SURFACE;
                }
                if (state->window && state->eglContext != EGL_NO_CONTEXT) {
                    EGLint surfAttrs[] = { EGL_NONE };
                    state->eglSurface = eglCreateWindowSurface(
                        state->eglDisplay, state->eglConfig, state->window, surfAttrs);
                    if (state->eglSurface != EGL_NO_SURFACE) {
                        eglMakeCurrent(state->eglDisplay, state->eglSurface,
                                       state->eglSurface, state->eglContext);
                        LOGI("EGL surface recreated");
                    }
                }
            }
        }
    }
}

void ApiRender::pollEvents() {
    /* На Android события обрабатываются в android_main */
}

void ApiRender::registerAppCallbacks(android_app* app) {
    m_androidState->app = app;
    app->onAppCmd = handleCmd;
    LOGI("APP_CMD callbacks registered");
}

int32_t ApiRender::getWidth() const {
    return m_androidState->width;
}

int32_t ApiRender::getHeight() const {
    return m_androidState->height;
}

#if defined(BEVOID_PLATFORM_ANDROID)
void* ApiRender::getAndroidActivity() const {
    return m_androidState->activity;
}
void* ApiRender::getAndroidWindow() const {
    return m_androidState->window;
}
void* ApiRender::getEGLDisplay() const {
    return m_androidState->eglDisplay;
}
void* ApiRender::getEGLSurface() const {
    return m_androidState->eglSurface;
}
void* ApiRender::getEGLContext() const {
    return m_androidState->eglContext;
}
void* ApiRender::getEGLConfig() const {
    return &m_androidState->eglConfig;
}
void ApiRender::setEGLSurface(void* surface) {
    m_androidState->eglSurface = reinterpret_cast<EGLSurface>(surface);
}
#endif

} // namespace com::bevoid::aporia::system

#endif /* BEVOID_PLATFORM_ANDROID */
