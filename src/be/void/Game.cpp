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
 * Чистый GLFW + накопление дельты мыши. F11 — фуллскрин.
 */

#include "be/void/Game.h"

/* Windows headers MUST come before GLFW to avoid APIENTRY redefinition */
#if defined(BEVOID_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

/* GLFW — только для десктопа */
#if !defined(BEVOID_PLATFORM_ANDROID)
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#else
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

#include <iostream>
#include <cmath>
#include <chrono>

namespace be::void_ {

static Game* g_game = nullptr;
static bool  g_mouseInputEnabled = true;

static double g_mouseAccumX = 0;
static double g_mouseAccumY = 0;
static float g_lastTouchX = 0;
static float g_lastTouchY = 0;

Game::Game() {
    g_game = this;
#if defined(BEVOID_PLATFORM_ANDROID)
    m_api = std::make_unique<com::bevoid::aporia::system::ApiRender>();
#endif
}

Game::~Game() { if (g_game == this) g_game = nullptr; }

bool Game::doInitOpenGL() { return initOpenGL(); }
bool Game::doInitCore()   { return m_core.init(); }
void Game::doRender() {
    int w = m_api ? m_api->getWidth()  : 1280;
    int h = m_api ? m_api->getHeight() : 720;
    m_core.render(m_time, w, h);
}
void Game::doShutdown()   { shutdown(); }

int Game::run(int /*argc*/, char** /*argv*/) {
    std::cout << "[Game] run() start\n";
    if (!initOpenGL()) {
        std::cerr << "[Game] Failed to initialize OpenGL\n";
        return 1;
    }
    std::cout << "[Game] OpenGL OK, calling Core::init()\n";
    if (!m_core.init()) {
        std::cerr << "[Game] Failed to init Core\n";
        shutdown();
        return 1;
    }
    std::cout << "[Game] Core OK, entering main loop\n";

    mainLoop();

    shutdown();
    std::cout << "[Game] Shutdown complete\n";
    return 0;
}

bool Game::initOpenGL() {
    if (!m_api) {
        m_api = std::make_unique<com::bevoid::aporia::system::ApiRender>();
    }

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

    glfwSetInputMode(m_api->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    int w, h;
    glfwGetWindowSize(m_api->getWindow(), &w, &h);

    static double lastX = w * 0.5, lastY = h * 0.5;
    glfwSetCursorPosCallback(m_api->getWindow(), [](GLFWwindow*, double x, double y) {
        if (g_mouseInputEnabled) {
            g_mouseAccumX += x - lastX;
            g_mouseAccumY += y - lastY;
        }
        lastX = x;
        lastY = y;
    });

    glfwSetKeyCallback(m_api->getWindow(), [](GLFWwindow* win, int key, int, int action, int) {
        if (g_game) g_game->m_core.getInput().onKey(key, action);

        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            if (g_mouseInputEnabled) {
                g_mouseInputEnabled = false;
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
            if (g_game && g_game->getApi()) {
                g_game->getApi()->toggleFullscreen();
            }
        }
    });

    glfwSetMouseButtonCallback(m_api->getWindow(), [](GLFWwindow* win, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !g_mouseInputEnabled) {
            if (g_game && !g_game->m_inMenu) {
                g_mouseInputEnabled = true;
                glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
                int ww, hh;
                glfwGetWindowSize(win, &ww, &hh);
                glfwSetCursorPos(win, ww * 0.5, hh * 0.5);
            }
        }
    });
#endif

    const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glVen = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* glRen = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

#if !defined(BEVOID_PLATFORM_ANDROID)
    std::cout << "[Game] OpenGL " << (glVer ? glVer : "?") << "\n";
    std::cout << "[Game] Vendor  : " << (glVen ? glVen : "?") << "\n";
    std::cout << "[Game] Renderer: " << (glRen ? glRen : "?") << "\n";
#endif

    m_api->getOsManager().getInfo().setGpuInfo(glVen, glRen, glVer);

    glEnable(GL_DEPTH_TEST);

    m_screenMgr.setCore(&m_core);
    m_screenMgr.setScreen(m_screenMgr.createScreen(screens::ScreenID::Menu));

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
    auto lastTick = std::chrono::steady_clock::now();

    while (m_running && !m_api->shouldClose()) {
        m_api->pollEvents();

        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - lastTick).count();
        if (delta > 0.1f) delta = 0.1f;
        lastTick = now;
        m_time += delta;

        bool wasInMenu = m_inMenu;
        m_inMenu = (m_screenMgr.currentScreen() &&
                     m_screenMgr.currentScreen()->id() != screens::ScreenID::Game);

        if (!m_inMenu) {
            if (g_mouseInputEnabled && (std::abs(g_mouseAccumX) > 0.01 || std::abs(g_mouseAccumY) > 0.01)) {
                m_core.getInput().onMouseMove(g_mouseAccumX, g_mouseAccumY);
                g_mouseAccumX = 0;
                g_mouseAccumY = 0;
            }
        }

        m_screenMgr.update(delta);

        bool nowInMenu = (m_screenMgr.currentScreen() &&
                           m_screenMgr.currentScreen()->id() != screens::ScreenID::Game);

        if (wasInMenu && !nowInMenu) {
            g_mouseInputEnabled = true;
            glfwSetInputMode(m_api->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(m_api->getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }
        if (!wasInMenu && nowInMenu) {
            g_mouseInputEnabled = false;
            glfwSetInputMode(m_api->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        m_inMenu = nowInMenu;

        if (m_inMenu) {
            m_screenMgr.render(m_time);
        } else {
            int w = m_api->getWidth();
            int h = m_api->getHeight();
            m_core.render(m_time, w, h);
        }

        m_api->swapBuffers();
    }
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
#endif

/* ============================================================
 * Entry point — Android
 * ============================================================ */
#if defined(BEVOID_PLATFORM_ANDROID)
#include "android_native_app_glue.h"
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define ALOG_TAG "BEVoid"
#define ALOGI(...) __android_log_print(ANDROID_LOG_INFO, ALOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR, ALOG_TAG, __VA_ARGS__)

static be::void_::Game* g_androidGame = nullptr;
static bool g_running = false;

static int32_t handleInput(android_app*, AInputEvent* event) {
    if (!g_androidGame) return 0;
    int32_t type = AInputEvent_getType(event);
    if (type == AINPUT_EVENT_TYPE_KEY) {
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t action = AKeyEvent_getAction(event);
        g_androidGame->getCore().getInput().onKey(keyCode, action);

        auto& ts = be::void_::screens::getTouchState();
        if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_DOWN) {
            ts.backPressed = true;
            return 1;
        }
        if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_UP) {
            ts.backPressed = false;
            return 1;
        }
        return 1;
    }
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        int winW = g_androidGame->getApi()->getWidth();
        int winH = g_androidGame->getApi()->getHeight();

        auto& ts = be::void_::screens::getTouchState();
        ts.ndcX = (2.0f * x / (float)winW) - 1.0f;
        ts.ndcY = 1.0f - (2.0f * y / (float)winH);

        if (actionMasked == AMOTION_EVENT_ACTION_DOWN) {
            ts.touched = true;
            ts.tapped = true;
        } else if (actionMasked == AMOTION_EVENT_ACTION_UP) {
            ts.touched = false;
        } else if (actionMasked == AMOTION_EVENT_ACTION_MOVE) {
            if (!g_mouseInputEnabled && !g_androidGame->isInMenu()) {
                g_mouseAccumX += (x - g_lastTouchX) * 0.5f;
                g_mouseAccumY += (y - g_lastTouchY) * 0.5f;
            }
        }

        g_lastTouchX = x;
        g_lastTouchY = y;
        return 1;
    }
    return 0;
}

extern "C" void android_main(struct android_app* app) {
    ALOGI("=== BEVoid Android entry point ===");

    be::void_::Game game;
    g_androidGame = &game;
    g_running = true;

    game.getApi()->registerAppCallbacks(app);
    app->onInputEvent = handleInput;

    if (!game.doInitOpenGL()) { ALOGE("OpenGL init failed"); return; }
    if (!game.doInitCore())   { ALOGE("Core init failed"); return; }

    ALOGI("Starting main loop");
    auto lastTick = std::chrono::steady_clock::now();

    while (g_running) {
        int events;
        struct android_poll_source* source;

        int timeoutMs = game.getApi()->getHeight() > 0 ? 0 : -1;

        while (ALooper_pollAll(timeoutMs, nullptr, &events, (void**)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }
        }

        int winW = game.getApi()->getWidth();
        int winH = game.getApi()->getHeight();
        if (winW > 0 && winH > 0) {
            auto now = std::chrono::steady_clock::now();
            float delta = std::chrono::duration<float>(now - lastTick).count();
            if (delta > 0.1f) delta = 0.1f;
            lastTick = now;

            game.addTime(delta);

            bool wasInMenu = game.isInMenu();
            bool nowInMenu = (game.getScreenMgr().currentScreen() != nullptr &&
                             game.getScreenMgr().currentScreen()->id() != screens::ScreenID::Game);
            game.setInMenu(nowInMenu);

            if (g_mouseInputEnabled && !nowInMenu) {
                if (std::abs(g_mouseAccumX) > 0.01 || std::abs(g_mouseAccumY) > 0.01) {
                    game.getCore().getInput().onMouseMove(g_mouseAccumX, g_mouseAccumY);
                    g_mouseAccumX = 0;
                    g_mouseAccumY = 0;
                }
            }

            game.getScreenMgr().update(delta);

            if (nowInMenu) {
                glViewport(0, 0, winW, winH);
                game.getScreenMgr().render(game.getTime());
            } else {
                game.getCore().render(game.getTime(), winW, winH);
            }

            game.getApi()->swapBuffers();
        }
    }

    ALOGI("Shutting down");
    game.doShutdown();
    g_androidGame = nullptr;
}
#endif
