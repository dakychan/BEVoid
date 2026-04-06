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
 * be.void.screens — MenuScreen
 *
 * Главное меню с текстом, кнопками, навигацией.
 */

#ifndef BEVOID_MENU_SCREEN_H
#define BEVOID_MENU_SCREEN_H

#include "screens/Screen.h"
#include <string>
#include <vector>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::screens {

struct MenuItem {
    std::string text;
    bool selected;
};

class MenuScreen : public Screen {
public:
    ScreenID id() const override { return ScreenID::Menu; }

    void onEnter() override;
    void onExit()  override;

    void update(float dt) override;
    void render(float time) override;

    ScreenID nextScreen() const override { return m_nextScreen; }

private:
    void drawButton(float y, const std::string& text, bool selected) const;
    void drawText(float x, float y, const std::string& text, float scale, float r, float g, float b) const;
    void initShaders();

    std::vector<MenuItem> m_items;
    int m_selected = 0;
    ScreenID m_nextScreen = ScreenID::None;

    GLuint m_buttonProg = 0;
    GLuint m_textProg = 0;
    bool m_shadersInit = false;
};

} // namespace be::void_::screens

#endif // BEVOID_MENU_SCREEN_H
