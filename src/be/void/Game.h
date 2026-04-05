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
 * Game — главный класс игры.
 * Платформо-независимый — использует ApiRender (GLFW + OpenGL).
 */

#ifndef BEVOID_GAME_H
#define BEVOID_GAME_H

#include "com/bevoid/aporia/system/ApiRender.h"
#include <memory>
#include <cstdint>

namespace be::void_ {

class Game {
public:
    Game();
    ~Game();

    /* Запустить игру */
    int run(int argc, char** argv);

private:
    bool   initOpenGL();
    bool   initShaders();
    bool   initGeometry();
    void   shutdown();
    void   mainLoop();
    void   update(float deltaTime);
    void   render();

    struct Impl;
    std::unique_ptr<Impl> pImpl;

    std::unique_ptr<com::bevoid::aporia::system::ApiRender> m_api;
    float m_time = 0.0f;
    bool  m_running = false;
};

} // namespace be::void_

#endif // BEVOID_GAME_H
