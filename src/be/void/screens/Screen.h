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
 * be.void.screens — Screen System
 *
 * Меню → Игра. Простой переключатель экранов.
 */

#ifndef BEVOID_SCREEN_H
#define BEVOID_SCREEN_H

#include <memory>

namespace be::void_ {
namespace screens {

enum class ScreenID {
    None,
    Menu,
    Game
};

class Screen {
public:
    virtual ~Screen() = default;
    virtual ScreenID id() const = 0;

    /* Вызываются при активации/деактивации */
    virtual void onEnter()  {}
    virtual void onExit()   {}

    /* Главный цикл */
    virtual void update(float dt) = 0;
    virtual void render(float time) = 0;

    /* Возвращает true = переключить экран */
    virtual ScreenID nextScreen() const { return ScreenID::None; }
};

class ScreenManager {
public:
    void setScreen(std::unique_ptr<Screen> screen);
    Screen* currentScreen() const;

    void update(float dt);
    void render(float time);

private:
    std::unique_ptr<Screen> m_screen;
};

} // namespace screens
} // namespace be::void_

#endif // BEVOID_SCREEN_H
