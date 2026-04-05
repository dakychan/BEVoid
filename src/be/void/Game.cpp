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
 * BEVoid Project — be.void (core game)
 *
 * Реализация Game: платформо-независимая.
 * ApiRender (GLFW + OpenGL) создаёт окно и контекст.
 * Шейдерный рендерер — VAO/VBO, вращающийся треугольник.
 */

#include "be/void/Game.h"

/* GLFW — только для десктопа */
#if !defined(BEVOID_PLATFORM_ANDROID)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#else
/* Android: OpenGL ES 3 напрямую */
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif

#include <iostream>
#include <cmath>
#include <vector>
#include <chrono>

#if defined(BEVOID_PLATFORM_WINDOWS)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

namespace be::void_ {

/* ============================================================
 * Шейдеры — GLSL 3.30 Core
 * ============================================================ */
static const char* VERT_SRC = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform float uTime;
void main() {
    vColor = aColor;
    float c = cos(uTime);
    float s = sin(uTime);
    vec2 p = mat2(c, -s, s, c) * aPos;
    gl_Position = vec4(p, 0.0, 1.0);
}
)";

static const char* FRAG_SRC = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)";

/* ============================================================
 * Геометрия — треугольник
 * ============================================================ */
static const float TRIANGLE_VERTS[] = {
    /*  pos       color       */
     0.0f,  0.6f,  1.0f, 0.2f, 0.3f,
    -0.5f, -0.4f,  0.2f, 0.8f, 0.3f,
     0.5f, -0.4f,  0.3f, 0.2f, 1.0f,
};
static const int TRIANGLE_COUNT = 3;
static const int VERTEX_STRIDE  = 5 * sizeof(float);

/* ============================================================
 * Impl — приватные данные
 * ============================================================ */
struct Game::Impl {
    GLuint vao       = 0;
    GLuint vbo       = 0;
    GLuint program   = 0;
    GLint  uTime     = -1;
    int    vertCount = 0;
};

Game::Game() : pImpl(std::make_unique<Impl>()) {}
Game::~Game() = default;

/* --- Public wrappers for Android entry point --- */
bool Game::doInitOpenGL()    { return initOpenGL(); }
bool Game::doInitShaders()   { return initShaders(); }
bool Game::doInitGeometry()  { return initGeometry(); }
void Game::doRender()        { render(); }
void Game::doShutdown()      { shutdown(); }

int Game::run(int /*argc*/, char** /*argv*/) {
    if (!initOpenGL()) {
        std::cerr << "[Game] Failed to initialize OpenGL\n";
        return 1;
    }
    if (!initShaders()) {
        std::cerr << "[Game] Failed to initialize shaders\n";
        shutdown();
        return 1;
    }
    if (!initGeometry()) {
        std::cerr << "[Game] Failed to initialize geometry\n";
        shutdown();
        return 1;
    }

    std::cout << "[Game] Starting main loop...\n";
    mainLoop();

    shutdown();
    std::cout << "[Game] Shutdown complete\n";
    return 0;
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "[Game] Shader compile error: " << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(p, 512, nullptr, log);
        std::cerr << "[Game] Program link error: " << log << "\n";
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

bool Game::initOpenGL() {
    /* Создаём ApiRender — GLFW+OpenGL (desktop) или EGL+GLES3 (Android) */
    m_api = std::make_unique<com::bevoid::aporia::system::ApiRender>();
    if (!m_api->create("BEVoid", 1280, 720)) {
        std::cerr << "[Game] ApiRender::create failed\n";
        return false;
    }

#if !defined(BEVOID_PLATFORM_ANDROID)
    /* Desktop: загружаем OpenGL функции через glad */
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "[Game] gladLoadGL failed\n";
        shutdown();
        return false;
    }
#endif

    /* GL Info — работает на всех платформах */
    const char* glVer = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    const char* glVen = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* glRen = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

#if !defined(BEVOID_PLATFORM_ANDROID)
    std::cout << "[Game] OpenGL " << (glVer ? glVer : "?") << "\n";
    std::cout << "[Game] Vendor  : " << (glVen ? glVen : "?") << "\n";
    std::cout << "[Game] Renderer: " << (glRen ? glRen : "?") << "\n";
#endif

    /* Обновляем OsManager GPU инфой */
    auto& info = const_cast<com::bevoid::aporia::system::SystemInfo&>(
        m_api->getOsManager().getInfo());
    info.gl_version   = glVer ? glVer : "N/A";
    info.gpu_vendor   = glVen ? glVen : "N/A";
    info.gpu_renderer = glRen ? glRen : "N/A";

    glEnable(GL_DEPTH_TEST);

#if !defined(BEVOID_PLATFORM_ANDROID)
    /* --- Render callback для перерисовки во время drag/resize --- */
    m_api->setRenderCallback([](void* userData) {
        auto* game = static_cast<Game*>(userData);
        game->render();
    }, this);
#endif

    m_running = true;
    return true;
}

bool Game::initShaders() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    if (!vs || !fs) return false;

    pImpl->program = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!pImpl->program) return false;

    pImpl->uTime = glGetUniformLocation(pImpl->program, "uTime");
    std::cout << "[Game] Shaders compiled OK\n";
    return true;
}

bool Game::initGeometry() {
    glGenVertexArrays(1, &pImpl->vao);
    glGenBuffers(1, &pImpl->vbo);

    pImpl->vertCount = TRIANGLE_COUNT;

    glBindVertexArray(pImpl->vao);
    glBindBuffer(GL_ARRAY_BUFFER, pImpl->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TRIANGLE_VERTS),
                 TRIANGLE_VERTS, GL_STATIC_DRAW);

    /* attrib 0: vec2 position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)0);

    /* attrib 1: vec3 color */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    std::cout << "[Game] Geometry created\n";
    return true;
}

void Game::shutdown() {
    if (pImpl->vao)  glDeleteVertexArrays(1, &pImpl->vao);
    if (pImpl->vbo)  glDeleteBuffers(1, &pImpl->vbo);
    if (pImpl->program) glDeleteProgram(pImpl->program);

    pImpl->vao = 0;
    pImpl->vbo = 0;
    pImpl->program = 0;

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

        update(delta);
        render();

        m_api->swapBuffers();
    }
#else
    /* Android: главный цикл в android_main (ApiRender.cpp) */
    /* Game::render() вызывается через callback */
#endif
}

void Game::update(float /*deltaTime*/) {
    /* Тут будет: ввод, физика, AI, сеть и т.д. */
}

void Game::render() {
    glClearColor(0.08f, 0.08f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(pImpl->program);
    glUniform1f(pImpl->uTime, m_time);

    glBindVertexArray(pImpl->vao);
    glDrawArrays(GL_TRIANGLES, 0, pImpl->vertCount);
    glBindVertexArray(0);
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
        case APP_CMD_TERM_WINDOW:
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
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY) {
        if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
            g_running = false;
            return 1;
        }
    }
    return 0;
}

extern "C" void android_main(struct android_app* app) {
    be::void_::Game game;
    g_game = &game;
    g_running = true;

    app->onAppCmd = handleCmd;
    app->onInputEvent = handleInput;

    /* Init OpenGL (EGL) via ApiRender */
    if (!game.doInitOpenGL()) {
        ALOGE("Failed to init OpenGL");
        return;
    }
    if (!game.doInitShaders()) {
        ALOGE("Failed to init shaders");
        return;
    }
    if (!game.doInitGeometry()) {
        ALOGE("Failed to init geometry");
        return;
    }

    /* Set up render callback */
    auto* api = game.getApi();
    api->setRenderCallback([](void*) {
        if (g_game) g_game->doRender();
    }, nullptr);

    /* Main loop */
    while (g_running) {
        int events;
        struct android_poll_source* source;

        while (ALooper_pollAll(g_hasWindow ? 0 : -1, nullptr,
                                &events, (void**)&source) >= 0) {
            if (source) source->process(app, source);
        }

        /* Create EGL surface если окно готово */
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

            /* Render */
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
