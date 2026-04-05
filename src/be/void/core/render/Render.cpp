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
 * be.void.core.render
 *
 * Реализация рендера — шейдеры + треугольник.
 */

#include "core/render/Render.h"
#include <iostream>
#include <cmath>

namespace be::void_::core::render {

/* ============================================================
 * Шейдеры — GLSL 3.30 Core / ES 3.0
 * ============================================================ */
#if defined(BEVOID_PLATFORM_ANDROID)
static const char* VERT_SRC = R"(
#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform float uTime;
void main() {
    vColor = aColor;
    float c = cos(uTime);
    float s = sin(uTime);
    vec2 p = mat2(c, -s, s, c) * aPos;
    gl_Position = vec4(p, 0.0, 1.0);
}
)";

static const char* FRAG_SRC = R"(
#version 300 es
precision mediump float;
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)";
#else
static const char* VERT_SRC = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform float uTime;
void main() {
    vColor = aColor;
    float c = cos(uTime);
    float s = sin(uTime);
    vec2 p = mat2(c, -s, s, c) * aPos;
    gl_Position = vec4(p, 0.0, 1.0);
}
)";

static const char* FRAG_SRC = R"(
#version 330 core
in vec3 vColor;
out vec4 fragColor;
void main() {
    fragColor = vec4(vColor, 1.0);
}
)";
#endif

/* ============================================================
 * Геометрия — треугольник
 * ============================================================ */
static const float TRIANGLE_VERTS[] = {
    /*  pos       color       */
     0.0f,  0.6f,  1.0f, 0.2f, 0.3f,
    -0.5f, -0.4f,  0.2f, 0.8f, 0.3f,
     0.5f, -0.4f,  0.3f, 0.2f, 1.0f,
};
static const int VERTEX_STRIDE = 5 * sizeof(float);

/* ============================================================
 * Хелперы
 * ============================================================ */
static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "[Render] Shader compile error: " << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static GLuint linkProgram(GLuint vs, GLuint fs) {
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(p, 512, nullptr, log);
        std::cerr << "[Render] Program link error: " << log << "\n";
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

/* ============================================================
 * Реализация
 * ============================================================ */
Render::Render() = default;
Render::~Render() = default;

bool Render::initShaders() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERT_SRC);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAG_SRC);
    if (!vs || !fs) return false;

    m_program = linkProgram(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!m_program) return false;

    m_uTime = glGetUniformLocation(m_program, "uTime");
    std::cout << "[Render] Shaders compiled OK\n";
    return true;
}

bool Render::initGeometry() {
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    m_vertCount = 3;

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(TRIANGLE_VERTS),
                 TRIANGLE_VERTS, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)(2 * sizeof(float)));

    glBindVertexArray(0);
    std::cout << "[Render] Geometry created\n";
    return true;
}

void Render::shutdown() {
    if (m_vao)  glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)  glDeleteBuffers(1, &m_vbo);
    if (m_program) glDeleteProgram(m_program);
    m_vao = 0; m_vbo = 0; m_program = 0; m_uTime = -1;
}

void Render::draw(float time) {
    glUseProgram(m_program);
    glUniform1f(m_uTime, time);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
    glBindVertexArray(0);
}

} // namespace be::void_::core::render
