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
#include "core/render/font/FontRenderer.h"
#include <string>
#include <vector>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
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
    void drawButton(float y, const std::string& text, bool hovered) const;
    void drawOutline(float x1, float y1, float x2, float y2, float r, float g, float b) const;
    void drawMsdfText(float x, float y, const std::string& text, float scale,
                      float r, float g, float b, float a = 1.0f);
    void initShaders();
    void selectItem(int idx);

    std::vector<MenuItem> m_items;
    ScreenID m_nextScreen = ScreenID::None;

    GLuint m_buttonProg = 0;
    GLuint m_textProg = 0;
    bool m_shadersInit = false;

    core::render::font::FontRenderer m_fontRenderer;
    bool m_fontsLoaded = false;

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    bool m_mouseLeftPressed = false;
    int m_hoveredItem = -1;

    static constexpr float BTN_X1 = -0.14f;
    static constexpr float BTN_X2 =  0.14f;
    static constexpr float BTN_HALF_H = 0.045f;
    static constexpr float BTN_SPACING = 0.12f;
    static constexpr float BTN_START_Y = 0.25f;
    static constexpr float TITLE_Y = 0.6f;
    static constexpr float TITLE_SCALE = 0.06f;
    static constexpr float BTN_TEXT_SCALE = 0.025f;
    static constexpr float OUTLINE_W = 0.003f;
};

} // namespace be::void_::screens

#endif // BEVOID_MENU_SCREEN_H
