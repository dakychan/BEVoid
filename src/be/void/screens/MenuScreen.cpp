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
#else
    #include <GLES3/gl3.h>
#endif

namespace be::void_::screens {

void MenuScreen::onEnter() {
    m_items = {
        { "Играть", false },
        { "Создать мир", false },
        { "Настройки", false },
        { "Выход", false },
    };
    m_selected = 0;
    m_nextScreen = ScreenID::None;
    initShaders();
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
    // TODO: Input через InputManager
}

void MenuScreen::render(float time) {
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    float pulse = 0.5f + 0.5f * std::sin(time * 3.0f);

    drawText(0.0f, 0.7f, "BEVoid", 0.08f, 0.2f + pulse * 0.3f, 0.6f + pulse * 0.2f, 0.9f);

    for (size_t i = 0; i < m_items.size(); i++) {
        float y = 0.4f - i * 0.15f;
        drawButton(y, m_items[i].text, (int)i == m_selected);
    }

    drawText(0.0f, -0.85f, "WASD - navigation | Enter - select", 0.02f, 0.5f, 0.5f, 0.5f);

    glEnable(GL_DEPTH_TEST);
}

void MenuScreen::initShaders() {
    if (m_shadersInit) return;

    const char* btnVs = R"(
#version 330 core
layout(0) in vec2 p;
layout(1) in vec3 c;
out vec3 vc;
void main(){gl_Position=vec4(p,0,1);vc=c;}
)";
    const char* btnFs = R"(
#version 330 core
in vec3 vc;
out vec4 fc;
void main(){fc=vec4(vc,1);}
)";

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &btnVs, nullptr);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &btnFs, nullptr);
    glCompileShader(f);
    m_buttonProg = glCreateProgram();
    glAttachShader(m_buttonProg, v);
    glAttachShader(m_buttonProg, f);
    glLinkProgram(m_buttonProg);
    glDeleteShader(v);
    glDeleteShader(f);

    m_textProg = m_buttonProg;
    m_shadersInit = true;
}

void MenuScreen::drawButton(float y, const std::string& text, bool selected) const {
    float bx1 = -0.4f, bx2 = 0.4f;
    float by1 = y - 0.06f, by2 = y + 0.06f;

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
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    drawText(0.0f, y, text, 0.04f, 1.0f, 1.0f, 1.0f);
}

void MenuScreen::drawText(float x, float y, const std::string& text, float scale, float r, float g, float b) const {
    float curX = x - (text.size() * scale * 0.3f);
    for (char c : text) {
        float x1 = curX, x2 = curX + scale;
        float y1 = y - scale * 0.5f, y2 = y + scale * 0.5f;

        float verts[] = {
            x1, y1, r, g, b,
            x2, y1, r, g, b,
            x2, y2, r, g, b,
            x1, y2, r, g, b,
        };
        uint32_t idx[] = { 0,1,2, 2,3,0 };

        static GLuint tvao = 0, tvbo = 0, tebo = 0;
        if (!tvao) {
            glGenVertexArrays(1, &tvao);
            glGenBuffers(1, &tvbo);
            glGenBuffers(1, &tebo);
            glBindVertexArray(tvao);
            glBindBuffer(GL_ARRAY_BUFFER, tvbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
            glBindVertexArray(0);
        }

        glUseProgram(m_textProg);
        glBindBuffer(GL_ARRAY_BUFFER, tvbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glBindVertexArray(tvao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        curX += scale * 0.6f;
    }
}

} // namespace be::void_::screens
