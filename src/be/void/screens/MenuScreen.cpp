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
 * Рисует кнопки в NDC через простые VAO.
 * Enter = выбрать, WASD/стрелки = навигация.
 */

#include "screens/MenuScreen.h"
#include <iostream>
#include <cstring>

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
        { "Выход", false },
    };
    m_selected = 0;
    m_nextScreen = ScreenID::None;
    std::cout << "[Menu] Enter\n";
}

void MenuScreen::onExit() {
    std::cout << "[Menu] Exit\n";
}

void MenuScreen::update(float /*dt*/) {
    /* Простой ввод — в реальном проекте будет InputManager */
}

void MenuScreen::render(float /*time*/) {
    glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Заголовок */
    glDisable(GL_DEPTH_TEST);

    /* Рисуем кнопки */
    for (size_t i = 0; i < m_items.size(); i++) {
        drawButton(0.4f - i * 0.2f, m_items[i].text, (int)i == m_selected);
    }

    glEnable(GL_DEPTH_TEST);
}

void MenuScreen::drawButton(float y, const std::string& text, bool selected) const {
    /* Квадрат кнопки */
    float bx1 = -0.3f, bx2 = 0.3f;
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

    static GLuint vao = 0, vbo = 0, ebo = 0, prog = 0;
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

        const char* vs = "#version 330 core\nlayout(0) in vec2 p;layout(1) in vec3 c;out vec3 vc;void main(){gl_Position=vec4(p,0,1);vc=c;}";
        const char* fs = "#version 330 core\nin vec3 vc;out vec4 fc;void main(){fc=vec4(vc,1);}";
        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &vs, nullptr); glCompileShader(v);
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &fs, nullptr); glCompileShader(f);
        prog = glCreateProgram();
        glAttachShader(prog, v); glAttachShader(prog, f);
        glLinkProgram(prog);
        glDeleteShader(v); glDeleteShader(f);
    }

    glUseProgram(prog);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace be::void_::screens
