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
#include "screens/Screen.h"
#include "com/bevoid/aporia/system/ApiRender.h"
#include <memory>

namespace be::void_ {

class Game {
public:
    Game();
    ~Game();

    int run(int argc, char** argv);

    bool doInitOpenGL();
    bool doInitCore();
    void doRender();
    void doShutdown();

    core::Core& getCore()  { return m_core;  }
    com::bevoid::aporia::system::ApiRender* getApi() { return m_api.get(); }

    screens::ScreenManager& getScreenMgr() { return m_screenMgr; }
    float getTime() const { return m_time; }
    bool isInMenu() const { return m_inMenu; }

    void addTime(float dt) { m_time += dt; }
    void setInMenu(bool v) { m_inMenu = v; }

private:
    bool   initOpenGL();
    void   shutdown();
    void   mainLoop();

    std::unique_ptr<com::bevoid::aporia::system::ApiRender> m_api;
    core::Core m_core;
    screens::ScreenManager m_screenMgr;
    float  m_time = 0.0f;
    bool   m_running = false;
    bool   m_inMenu = true;
};

} // namespace be::void_

#endif // BEVOID_GAME_H
