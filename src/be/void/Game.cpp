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
 * be.void — Game
 *
 * Платформо-независимая реализация.
 * ApiRender создаёт окно/контекст, Core управляет подсистемами.
 */

#include "be/void/Game.h"

#if defined(BEVOID_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/* GLFW — только для десктопа */
#if !defined(BEVOID_PLATFORM_ANDROID)
#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <glad/glad.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

#include <iostream>
#include <cmath>
#include <chrono>

namespace be::void_ {

/* Глобальные указатели для Win32 callback */
static Game* g_game = nullptr;
static bool  g_mouseInputEnabled = true;

#if defined(BEVOID_PLATFORM_WINDOWS)
static HWND g_hwnd = nullptr;
static void* g_oldWndProc = nullptr;

LRESULT CALLBACK bevoidWndProc(void* hwnd, unsigned int msg, unsigned __int64 wParam, __int64 lParam) {
    switch (msg) {
        case WM_INPUT: {
            if (!g_game) break;
            static bool first = true;
            if (first) { std::cerr << "[Input] WM_INPUT received! mode=" << g_mouseInputEnabled << "\n"; first = false; }
            if (!g_mouseInputEnabled) break;
            RAWINPUT raw;
            UINT size = sizeof(raw);
            UINT ret = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, &raw, &size, sizeof(RAWINPUT));
            if (ret == (UINT)-1) {
                static bool logged = false;
                if (!logged) { std::cerr << "[Input] GetRawInputData failed: " << GetLastError() << "\n"; logged = true; }
                break;
            }
            if (raw.header.dwType == RIM_TYPEMOUSE) {
                long dx = (long)raw.data.mouse.lLastX;
                long dy = (long)raw.data.mouse.lLastY;
                if (dx != 0 || dy != 0) {
                    g_game->getCore().getInput().onMouseMove((double)dx, -(double)dy);
                }
            }
            break;
        }
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE && g_game) {
                g_mouseInputEnabled = false;
                glfwSetInputMode(g_game->getApi()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                ShowCursor(TRUE);
            }
            if (g_game) {
                int key = 0;
                switch (wParam) {
                    case 'W': key = 87; break;
                    case 'S': key = 83; break;
                    case 'A': key = 65; break;
                    case 'D': key = 68; break;
                    case VK_SPACE: key = 32; break;
                    case VK_UP: key = 265; break;
                    case VK_DOWN: key = 264; break;
                    case VK_LEFT: key = 263; break;
                    case VK_RIGHT: key = 262; break;
                }
                if (key) g_game->getCore().getInput().onKey(key, 1);
            }
            break;
        case WM_KEYUP:
            if (g_game) {
                int key = 0;
                switch (wParam) {
                    case 'W': key = 87; break;
                    case 'S': key = 83; break;
                    case 'A': key = 65; break;
                    case 'D': key = 68; break;
                    case VK_SPACE: key = 32; break;
                }
                if (key) g_game->getCore().getInput().onKey(key, 0);
            }
            break;
        case WM_LBUTTONDOWN:
            if (g_game && !g_mouseInputEnabled) {
                g_mouseInputEnabled = true;
                glfwSetInputMode(g_game->getApi()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                ShowCursor(FALSE);
            }
            break;
        case WM_DESTROY:
            ShowCursor(TRUE);
            break;
    }
    return CallWindowProc((WNDPROC)g_oldWndProc, (HWND)hwnd, msg, (WPARAM)wParam, (LPARAM)lParam);
}
#endif

Game::Game() { g_game = this; }
Game::~Game() { if (g_game == this) g_game = nullptr; }

/* --- Public wrappers for Android --- */
bool Game::doInitOpenGL() { return initOpenGL(); }
bool Game::doInitCore()   { return m_core.init(); }
void Game::doRender()     { m_core.render(m_time); }
void Game::doShutdown()   { shutdown(); }

int Game::run(int /*argc*/, char** /*argv*/) {
    if (!initOpenGL()) {
        std::cerr << "[Game] Failed to initialize OpenGL\n";
        return 1;
    }
    if (!m_core.init()) {
        std::cerr << "[Game] Failed to init Core\n";
        shutdown();
        return 1;
    }

    std::cout << "[Game] Starting main loop...\n";
    mainLoop();

    shutdown();
    std::cout << "[Game] Shutdown complete\n";
    return 0;
}

bool Game::initOpenGL() {
    m_api = std::make_unique<com::bevoid::aporia::system::ApiRender>();
    if (!m_api->create("BEVoid", 1280, 720)) {
        std::cerr << "[Game] ApiRender::create failed\n";
        return false;
    }

#if !defined(BEVOID_PLATFORM_ANDROID)
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[Game] gladLoadGL failed\n";
        shutdown();
        return false;
    }

    /* --- Собственный ввод: Win32 Raw Input + subclass --- */
#if defined(BEVOID_PLATFORM_WINDOWS)
    HWND hwnd = glfwGetWin32Window(m_api->getWindow());
    g_hwnd = hwnd;

    /* Subclass оконной процедуры */
    g_oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)bevoidWndProc);

    /* Регистрируем Raw Input для мыши */
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;  /* Generic Desktop */
    rid.usUsage     = 0x02;  /* Mouse */
    rid.dwFlags     = RIDEV_INPUTSINK;
    rid.hwndTarget  = hwnd;
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        std::cerr << "[Game] RegisterRawInputDevices failed\n";
    }

    /* Старт с захваченной мышью */
    glfwSetInputMode(m_api->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    ShowCursor(FALSE);

    /* ЛКМ — повторный захват (в subclass) */
    /* ESC — освобождение (в subclass) */
    /* Клавиатура — через subclass WM_KEYDOWN/WM_KEYUP */
#endif /* WINDOWS */
#endif /* !ANDROID */

    const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glVen = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* glRen = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

#if !defined(BEVOID_PLATFORM_ANDROID)
    std::cout << "[Game] OpenGL " << (glVer ? glVer : "?") << "\n";
    std::cout << "[Game] Vendor  : " << (glVen ? glVen : "?") << "\n";
    std::cout << "[Game] Renderer: " << (glRen ? glRen : "?") << "\n";
#endif

    auto& info = const_cast<com::bevoid::aporia::system::SystemInfo&>(
        m_api->getOsManager().getInfo());
    info.gl_version   = glVer ? glVer : "N/A";
    info.gpu_vendor   = glVen ? glVen : "N/A";
    info.gpu_renderer = glRen ? glRen : "N/A";

    glEnable(GL_DEPTH_TEST);

#if !defined(BEVOID_PLATFORM_ANDROID)
    m_api->setRenderCallback([](void* userData) {
        auto* game = static_cast<Game*>(userData);
        game->m_core.render(game->m_time);
    }, this);
#endif

    m_running = true;
    return true;
}

void Game::shutdown() {
    m_core.shutdown();
    if (m_api) {
        m_api->shutdown();
        m_api.reset();
    }
    m_running = false;
}

void Game::mainLoop() {
#if !defined(BEVOID_PLATFORM_ANDROID)
    std::cerr << "[Game] mainLoop() STARTED\n";

    auto lastTick = std::chrono::steady_clock::now();

    while (m_running && !m_api->shouldClose()) {
        m_api->pollEvents();

        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - lastTick).count();
        if (delta > 0.1f) delta = 0.1f;
        lastTick = now;
        m_time += delta;

        m_core.update(delta);
        m_core.render(m_time);

        m_api->swapBuffers();
    }

    std::cerr << "[Game] mainLoop() EXIT\n";
#else
    /* Android: главный цикл в android_main */
#endif
}

} // namespace be::void_

/* ============================================================
 * Entry point — Desktop
 * ============================================================ */
#if !defined(BEVOID_PLATFORM_ANDROID)
#if defined(BEVOID_PLATFORM_WINDOWS)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    be::void_::Game game;
    return game.run(0, nullptr);
}
#else
int main(int argc, char** argv) {
    be::void_::Game game;
    return game.run(argc, argv);
}
#endif
#endif /* !BEVOID_PLATFORM_ANDROID */

/* ============================================================
 * Entry point — Android (NativeActivity)
 * ============================================================ */
#if defined(BEVOID_PLATFORM_ANDROID)
#include "android_native_app_glue.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define ALOG_TAG "BEVoid"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, ALOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, ALOG_TAG, __VA_ARGS__)

static be::void_::Game* g_game = nullptr;
static bool g_running = false;
static bool g_hasWindow = false;
static ANativeWindow* g_window = nullptr;

static void handleCmd(android_app* app, int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            g_window = app->window;
            g_hasWindow = true;
            ALOGI("Window initialized");
            break;
        case APP_CMD_TERM_WINDOW: {
            if (g_game) {
                auto* api = g_game->getApi();
                if (api) {
                    EGLDisplay dpy = reinterpret_cast<EGLDisplay>(api->getEGLDisplay());
                    EGLSurface surf = reinterpret_cast<EGLSurface>(api->getEGLSurface());
                    if (dpy && surf) {
                        eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                        eglDestroySurface(dpy, surf);
                        api->setEGLSurface(nullptr);
                    }
                }
            }
            g_window = nullptr;
            g_hasWindow = false;
            ALOGI("Window terminated");
            break;
        }
        case APP_CMD_GAINED_FOCUS:
            break;
        case APP_CMD_LOST_FOCUS:
            break;
        case APP_CMD_DESTROY:
            g_running = false;
            break;
    }
}

static int32_t handleInput(android_app* /*app*/, AInputEvent* event) {
    int32_t type = AInputEvent_getType(event);
    if (!g_game) return 0;

    if (type == AINPUT_EVENT_TYPE_KEY) {
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t action  = AKeyEvent_getAction(event);
        g_game->getCore().getInput().onKey(keyCode, action);

        if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_UP) {
            g_running = false;
            return 1;
        }
        return 1;
    }
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        g_game->getCore().getInput().onMouseMove(x * 0.01f, y * 0.01f);
        return 1;
    }
    return 0;
}

extern "C" void android_main(struct android_app* app) {
    be::void_::Game game;
    g_game = &game;
    g_running = true;

    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    if (!game.doInitOpenGL()) { ALOGE("Failed to init OpenGL"); return; }
    if (!game.doInitCore())   { ALOGE("Failed to init Core");   return; }

    auto* api = game.getApi();
    api->setRenderCallback([](void*) {
        if (g_game) g_game->doRender();
    }, nullptr);

    while (g_running) {
        int events;
        struct android_poll_source* source;

        while (ALooper_pollAll(g_hasWindow ? 0 : -1, nullptr,
                                &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        if (g_hasWindow && g_window) {
            EGLDisplay dpy = reinterpret_cast<EGLDisplay>(api->getEGLDisplay());
            EGLSurface surf = reinterpret_cast<EGLSurface>(api->getEGLSurface());
            EGLConfig* cfg = reinterpret_cast<EGLConfig*>(api->getEGLConfig());
            EGLContext ctx = reinterpret_cast<EGLContext>(api->getEGLContext());

            if (surf == EGL_NO_SURFACE && dpy && cfg && ctx) {
                surf = eglCreateWindowSurface(dpy, *cfg, g_window, nullptr);
                if (surf == EGL_NO_SURFACE) {
                    ALOGE("eglCreateWindowSurface failed: 0x%x", eglGetError());
                } else {
                    if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
                        ALOGE("eglMakeCurrent failed: 0x%x", eglGetError());
                    } else {
                        int32_t w = ANativeWindow_getWidth(g_window);
                        int32_t h = ANativeWindow_getHeight(g_window);
                        glViewport(0, 0, w, h);
                        api->setEGLSurface(surf);
                        ALOGI("EGL surface created: %dx%d", w, h);
                    }
                }
            }

            if (surf != EGL_NO_SURFACE) {
                api->callRenderCallback();
                eglSwapBuffers(dpy, surf);
            }
        }
    }

    game.doShutdown();
    g_game = nullptr;
    ALOGI("Android main exited");
}
#endif
