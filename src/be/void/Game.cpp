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
 * Чистый GLFW + накопление дельты мыши. Никакого Raw Input.
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

/* Глобальный указатель */
static Game* g_game = nullptr;
static bool  g_mouseInputEnabled = true;

/* Накапливаем дельту мыши в колбэке */
static double g_mouseAccumX = 0;
static double g_mouseAccumY = 0;

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

    /* --- Мышь: GLFW callback с накоплением дельты --- */
    glfwSetInputMode(m_api->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetInputMode(m_api->getWindow(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

    /* Центрируем курсор */
    int w, h;
    glfwGetWindowSize(m_api->getWindow(), &w, &h);
    glfwSetCursorPos(m_api->getWindow(), w * 0.5, h * 0.5);

    static double lastX = w * 0.5, lastY = h * 0.5;
    glfwSetCursorPosCallback(m_api->getWindow(), [](GLFWwindow*, double x, double y) {
        if (g_mouseInputEnabled) {
            g_mouseAccumX += x - lastX;
            g_mouseAccumY += y - lastY;
        }
        lastX = x;
        lastY = y;
    });

    /* Клавиатура */
    glfwSetKeyCallback(m_api->getWindow(), [](GLFWwindow* win, int key, int, int action, int) {
        if (g_game) g_game->m_core.getInput().onKey(key, action);

        /* ESC — освободить курсор */
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            g_mouseInputEnabled = false;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            int ww, hh;
            glfwGetWindowSize(win, &ww, &hh);
            glfwSetCursorPos(win, ww * 0.5, hh * 0.5);
        }
    });

    /* ЛКМ — захватить снова */
    glfwSetMouseButtonCallback(m_api->getWindow(), [](GLFWwindow* win, int button, int action, int) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !g_mouseInputEnabled) {
            g_mouseInputEnabled = true;
            glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetInputMode(win, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
            int ww, hh;
            glfwGetWindowSize(win, &ww, &hh);
            glfwSetCursorPos(win, ww * 0.5, hh * 0.5);
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

    auto& info = const_cast<com::bevoid::aporia::system::SystemInfo&>(
        m_api->getOsManager().getInfo());
    info.setGpuInfo(glVen, glRen, glVer);

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
    auto lastTick = std::chrono::steady_clock::now();

    while (m_running && !m_api->shouldClose()) {
        m_api->pollEvents();

        /* --- Обрабатываем накопленную дельту мыши --- */
        if (g_mouseInputEnabled && (std::abs(g_mouseAccumX) > 0.01 || std::abs(g_mouseAccumY) > 0.01)) {
            m_core.getInput().onMouseMove(g_mouseAccumX, g_mouseAccumY);
            g_mouseAccumX = 0;
            g_mouseAccumY = 0;
        }

        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - lastTick).count();
        if (delta > 0.1f) delta = 0.1f;
        lastTick = now;
        m_time += delta;

        m_core.update(delta);
        m_core.render(m_time);

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
        if (keyCode == AKEYCODE_BACK && action == AKEY_EVENT_ACTION_PRESS) {
            g_running = false;
            return 1;
        }
        return 1;
    }
    if (type == AINPUT_EVENT_TYPE_MOTION) {
        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);
        g_androidGame->getCore().getInput().onMouseMove(x * 0.01f, y * 0.01f);
        return 1;
    }
    return 0;
}

extern "C" void android_main(struct android_app* app) {
    ALOGI("=== BEVoid Android entry point ===");
    
    be::void_::Game game;
    g_androidGame = &game;
    g_running = true;
    
    /* Привязываем APP_CMD callbacks до инициализации */
    game.getApi()->registerAppCallbacks(app);
    app->onInputEvent = handleInput;
    
    /* Инициализация */
    if (!game.doInitOpenGL()) { ALOGE("OpenGL init failed"); return; }
    if (!game.doInitCore())   { ALOGE("Core init failed"); return; }
    
    /* Render callback */
    game.getApi()->setRenderCallback([](void*) {
        if (g_androidGame) g_androidGame->doRender();
    }, nullptr);
    
    /* Главный цикл */
    ALOGI("Starting main loop");
    while (g_running) {
        int events;
        struct android_poll_source* source;
        
        /* Блокируем только если нет окна, иначе не блокируем */
        int timeoutMs = g_androidGame->getApi()->getHeight() > 0 ? 0 : -1;
        
        while (ALooper_pollAll(timeoutMs, nullptr, &events, (void**)&source) >= 0) {
            if (source) {
                source->process(app, source);
            }
        }
        
        /* Рендерим только если окно активно */
        if (g_androidGame->getApi()->getHeight() > 0) {
            game.getApi()->callRenderCallback();
            game.getApi()->swapBuffers();
        }
    }
    
    ALOGI("Shutting down");
    game.doShutdown();
    g_androidGame = nullptr;
}
#endif
