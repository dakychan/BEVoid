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
 * be.void.screens — MenuScreen implementation
 *
 * Рисует кнопки + текст через простые шейдеры.
 * Enter = выбрать, WASD/стрелки = навигация.
 */

#include "screens/MenuScreen.h"
#include <iostream>
#include <cstring>
#include <cmath>

#if !defined(BEVOID_PLATFORM_ANDROID)
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#else
    #include <GLES3/gl3.h>
#endif

namespace be::void_::screens {

void MenuScreen::onEnter() {
    m_items = {
        { "Play", false },
        { "Create World", false },
        { "Settings", false },
        { "Exit", false },
    };
    m_selected = 0;
    m_nextScreen = ScreenID::None;
    initShaders();

    if (!m_fontsLoaded) {
        m_fontRenderer.init();
        m_fontRenderer.loadFont("default", "default");
        m_fontRenderer.loadFont("bold", "bold");
        m_fontsLoaded = true;
    }

    std::cout << "[Menu] Enter\n";
}

void MenuScreen::onExit() {
    if (m_buttonProg) glDeleteProgram(m_buttonProg);
    if (m_textProg) glDeleteProgram(m_textProg);
    m_buttonProg = 0;
    m_textProg = 0;
    std::cout << "[Menu] Exit\n";
}

void MenuScreen::update(float /*dt*/) {
    if (m_nextScreen != ScreenID::None) return;

#if !defined(BEVOID_PLATFORM_ANDROID)
    auto* window = glfwGetCurrentContext();

    glfwGetCursorPos(window, &m_mouseX, &m_mouseY);
    int winW, winH;
    glfwGetWindowSize(window, &winW, &winH);

    float ndcX = (2.0f * (float)m_mouseX / (float)winW) - 1.0f;
    float ndcY = 1.0f - (2.0f * (float)m_mouseY / (float)winH);

    m_hoveredItem = -1;
    for (size_t i = 0; i < m_items.size(); i++) {
        float btnY = BTN_START_Y - (float)i * BTN_SPACING;
        if (ndcX >= BTN_X1 && ndcX <= BTN_X2 &&
            ndcY >= btnY - BTN_HALF_H && ndcY <= btnY + BTN_HALF_H) {
            m_hoveredItem = (int)i;
            break;
        }
    }

    if (m_hoveredItem >= 0) {
        m_selected = m_hoveredItem;
    }

    bool leftNow = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    if (leftNow && !m_mouseLeftPressed && m_hoveredItem >= 0) {
        m_selected = m_hoveredItem;
        selectItem();
    }
    m_mouseLeftPressed = leftNow;

    bool upNow = glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS ||
                 glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS;
    bool downNow = glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS ||
                   glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS;
    bool enterNow = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS ||
                    glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS;

    if (upNow && !m_upPressed) {
        m_selected = (m_selected - 1 + (int)m_items.size()) % (int)m_items.size();
    }
    m_upPressed = upNow;

    if (downNow && !m_downPressed) {
        m_selected = (m_selected + 1) % (int)m_items.size();
    }
    m_downPressed = downNow;

    if (enterNow && !m_enterPressed) {
        selectItem();
    }
    m_enterPressed = enterNow;
#endif
}

void MenuScreen::selectItem() {
    switch (m_selected) {
        case 0: m_nextScreen = ScreenID::Game; break;
        case 1: break;
        case 2: break;
        case 3: break;
        default: break;
    }
}

void MenuScreen::render(float time) {
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    float pulse = 0.5f + 0.5f * std::sin(time * 3.0f);

    drawMsdfText(0.0f, TITLE_Y, "BEVoid", TITLE_SCALE,
                 0.2f + pulse * 0.3f, 0.6f + pulse * 0.2f, 0.9f);

    for (size_t i = 0; i < m_items.size(); i++) {
        float y = BTN_START_Y - (float)i * BTN_SPACING;
        drawButton(y, m_items[i].text, (int)i == m_selected);
    }

    drawMsdfText(0.0f, -0.85f, "WASD / Mouse - navigation | Enter / Click - select", 0.02f,
                 0.5f, 0.5f, 0.5f);

    glEnable(GL_DEPTH_TEST);
}

void MenuScreen::initShaders() {
    if (m_shadersInit) return;

    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetShaderInfoLog(s, 512, nullptr, log);
            std::cerr << "[Menu] Shader compile error: " << log << "\n";
            glDeleteShader(s);
            return 0;
        }
        return s;
    };

    const char* btnVs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
#else
        "#version 330 core\n"
#endif
        "layout(location = 0) in vec2 p;\n"
        "layout(location = 1) in vec3 c;\n"
        "out vec3 vc;\n"
        "void main(){gl_Position=vec4(p,0,1);vc=c;}\n";
    const char* btnFs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
        "precision mediump float;\n"
#else
        "#version 330 core\n"
#endif
        "in vec3 vc;\n"
        "out vec4 fc;\n"
        "void main(){fc=vec4(vc,1);}\n";

    GLuint v = compile(GL_VERTEX_SHADER, btnVs);
    GLuint f = compile(GL_FRAGMENT_SHADER, btnFs);
    if (!v || !f) {
        std::cerr << "[Menu] Shader compilation failed\n";
        m_shadersInit = true;
        return;
    }
    m_buttonProg = glCreateProgram();
    glAttachShader(m_buttonProg, v);
    glAttachShader(m_buttonProg, f);
    glLinkProgram(m_buttonProg);
    GLint linked;
    glGetProgramiv(m_buttonProg, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(m_buttonProg, 512, nullptr, log);
        std::cerr << "[Menu] Program link error: " << log << "\n";
        glDeleteProgram(m_buttonProg);
        m_buttonProg = 0;
    }
    glDeleteShader(v);
    glDeleteShader(f);

    m_textProg = m_buttonProg;
    m_shadersInit = true;
}

void MenuScreen::drawButton(float y, const std::string& text, bool selected) const {
    float bx1 = BTN_X1, bx2 = BTN_X2;
    float by1 = y - BTN_HALF_H, by2 = y + BTN_HALF_H;

    float r = selected ? 0.15f : 0.08f;
    float g = selected ? 0.6f : 0.12f;
    float b = selected ? 0.9f : 0.2f;

    float verts[] = {
        bx1, by1, r, g, b,
        bx2, by1, r, g, b,
        bx2, by2, r, g, b,
        bx1, by2, r, g, b,
    };
    uint32_t idx[] = { 0,1,2, 2,3,0 };

    static GLuint vao = 0, vbo = 0, ebo = 0;
    if (!vao) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glBindVertexArray(0);
    }

    glUseProgram(m_buttonProg);
    if (!m_buttonProg) return;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    const_cast<MenuScreen*>(this)->drawMsdfText(0.0f, y, text, BTN_TEXT_SCALE, 1.0f, 1.0f, 1.0f);
}

void MenuScreen::drawMsdfText(float x, float y, const std::string& text, float scale,
                               float r, float g, float b, float a) {
    if (!m_fontsLoaded) return;
    float tw = m_fontRenderer.getTextWidth("bold", text, scale);
    float startX = x - tw * 0.5f;
    m_fontRenderer.drawText("bold", text, startX, y, scale, r, g, b, a);
}

} // namespace be::void_::screens
