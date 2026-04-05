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
 * be.void — Game (тонкая обёртка над Core)
 *
 * Game управляет ApiRender (окно/контекст) и Core (подсистемы).
 */

#ifndef BEVOID_GAME_H
#define BEVOID_GAME_H

#include "core/Core.h"
#include "com/bevoid/aporia/system/ApiRender.h"
#include <memory>

namespace be::void_ {

class Game {
public:
    Game();
    ~Game();

    /* Запустить игру */
    int run(int argc, char** argv);

    /* Публичные методы для Android entry point */
    bool doInitOpenGL();
    bool doInitCore();
    void doRender();
    void doShutdown();

    /* Доступ к ApiRender для Android */
    com::bevoid::aporia::system::ApiRender* getApi() { return m_api.get(); }

private:
    bool   initOpenGL();
    void   shutdown();
    void   mainLoop();

    std::unique_ptr<com::bevoid::aporia::system::ApiRender> m_api;
    core::Core m_core;
    float  m_time = 0.0f;
    bool   m_running = false;
};

} // namespace be::void_

#endif // BEVOID_GAME_H
