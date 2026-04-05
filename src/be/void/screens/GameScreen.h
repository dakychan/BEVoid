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
 * be.void.screens — GameScreen
 *
 * Игровой экран — обёртка вокруг Core + Render.
 */

#ifndef BEVOID_GAME_SCREEN_H
#define BEVOID_GAME_SCREEN_H

#include "screens/Screen.h"
#include "core/Core.h"
#include "com/bevoid/aporia/system/ApiRender.h"
#include <memory>

namespace be::void_::screens {

class GameScreen : public Screen {
public:
    ScreenID id() const override { return ScreenID::Game; }

    void onEnter() override;
    void onExit()  override;

    void update(float dt) override;
    void render(float time) override;

    ScreenID nextScreen() const override { return m_nextScreen; }

public:
    std::unique_ptr<com::bevoid::aporia::system::ApiRender> m_api;
    core::Core m_core;
    bool m_initialized = false;
    float m_time = 0;
    ScreenID m_nextScreen = ScreenID::None;
};

} // namespace be::void_::screens

#endif // BEVOID_GAME_SCREEN_H
