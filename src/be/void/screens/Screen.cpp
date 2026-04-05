/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#include "screens/Screen.h"

namespace be::void_::screens {

void ScreenManager::setScreen(std::unique_ptr<Screen> screen) {
    if (m_screen) m_screen->onExit();
    m_screen = std::move(screen);
    if (m_screen) m_screen->onEnter();
}

Screen* ScreenManager::currentScreen() const {
    return m_screen.get();
}

void ScreenManager::update(float dt) {
    if (!m_screen) return;
    m_screen->update(dt);

    auto next = m_screen->nextScreen();
    if (next != ScreenID::None) {
        /* Переключение экрана — будет обработано извне */
    }
}

void ScreenManager::render(float time) {
    if (m_screen) m_screen->render(time);
}

} // namespace be::void_::screens
