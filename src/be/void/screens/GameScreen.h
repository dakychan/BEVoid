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

namespace be::void_ {
    namespace core { class Core; }
    namespace com::bevoid::aporia::system { class ApiRender; }
}

namespace be::void_::screens {

class GameScreen : public Screen {
public:
    ScreenID id() const override { return ScreenID::Game; }

    void onEnter() override;
    void onExit()  override;

    void update(float dt) override;
    void render(float time) override;

    ScreenID nextScreen() const override { return m_nextScreen; }

    void setCore(core::Core* core) { m_core = core; }

private:
    core::Core* m_core = nullptr;
    float m_time = 0;
    ScreenID m_nextScreen = ScreenID::None;
    bool m_escPressed = false;
};

} // namespace be::void_::screens

#endif // BEVOID_GAME_SCREEN_H
