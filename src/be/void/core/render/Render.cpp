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
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
uniform float uTime;
uniform mat4 uView;
uniform mat4 uProj;
void main() {
    vColor = aColor;
    float c = cos(uTime * 0.5);
    float s = sin(uTime * 0.5);
    vec3 p = aPos;
    /* Вращение вокруг Y для наглядности */
    p.xz = mat2(c, -s, s, c) * p.xz;
    gl_Position = uProj * uView * vec4(p, 1.0);
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
 * Геометрия — 3D куб (для наглядности движения)
 * ============================================================ */
static const float CUBE_VERTS[] = {
    /* pos          color        */
    /* Front face */
    -0.5f,-0.5f, 0.5f,  0.8f,0.2f,0.2f,
     0.5f,-0.5f, 0.5f,  0.8f,0.2f,0.2f,
     0.5f, 0.5f, 0.5f,  0.8f,0.2f,0.2f,
    -0.5f, 0.5f, 0.5f,  0.8f,0.2f,0.2f,
    /* Back face */
    -0.5f,-0.5f,-0.5f,  0.2f,0.8f,0.2f,
    -0.5f, 0.5f,-0.5f,  0.2f,0.8f,0.2f,
     0.5f, 0.5f,-0.5f,  0.2f,0.8f,0.2f,
     0.5f,-0.5f,-0.5f,  0.2f,0.8f,0.2f,
    /* Top face */
    -0.5f, 0.5f,-0.5f,  0.2f,0.2f,0.8f,
    -0.5f, 0.5f, 0.5f,  0.2f,0.2f,0.8f,
     0.5f, 0.5f, 0.5f,  0.2f,0.2f,0.8f,
     0.5f, 0.5f,-0.5f,  0.2f,0.2f,0.8f,
    /* Bottom face */
    -0.5f,-0.5f,-0.5f,  0.8f,0.8f,0.2f,
     0.5f,-0.5f,-0.5f,  0.8f,0.8f,0.2f,
     0.5f,-0.5f, 0.5f,  0.8f,0.8f,0.2f,
    -0.5f,-0.5f, 0.5f,  0.8f,0.8f,0.2f,
    /* Right face */
     0.5f,-0.5f, 0.5f,  0.8f,0.2f,0.8f,
     0.5f, 0.5f, 0.5f,  0.8f,0.2f,0.8f,
     0.5f, 0.5f,-0.5f,  0.8f,0.2f,0.8f,
     0.5f,-0.5f,-0.5f,  0.8f,0.2f,0.8f,
    /* Left face */
    -0.5f,-0.5f, 0.5f,  0.2f,0.8f,0.8f,
    -0.5f,-0.5f,-0.5f,  0.2f,0.8f,0.8f,
    -0.5f, 0.5f,-0.5f,  0.2f,0.8f,0.8f,
    -0.5f, 0.5f, 0.5f,  0.2f,0.8f,0.8f,
};
static const uint32_t CUBE_INDICES[] = {
    0,1,2, 2,3,0,       /* front */
    4,5,6, 6,7,4,       /* back */
    8,9,10, 10,11,8,    /* top */
    12,13,14, 14,15,12, /* bottom */
    16,17,18, 18,19,16, /* right */
    20,21,22, 22,23,20, /* left */
};
static constexpr int CUBE_INDEX_COUNT = 36;
static constexpr int VERTEX_STRIDE = 6 * sizeof(float);

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
    glGenBuffers(1, &m_ebo);

    m_indexCount = CUBE_INDEX_COUNT;

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTS),
                 CUBE_VERTS, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(CUBE_INDICES),
                 CUBE_INDICES, GL_STATIC_DRAW);

    /* attrib 0: vec3 position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)0);

    /* attrib 1: vec3 color */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          VERTEX_STRIDE, (void*)(3 * sizeof(float)));

    glBindVertexArray(0);

    m_uView = glGetUniformLocation(m_program, "uView");
    m_uProj  = glGetUniformLocation(m_program, "uProj");

    std::cout << "[Render] Geometry created (cube, indexed)\n";
    return true;
}

void Render::shutdown() {
    if (m_vao)  glDeleteVertexArrays(1, &m_vao);
    if (m_vbo)  glDeleteBuffers(1, &m_vbo);
    if (m_program) glDeleteProgram(m_program);
    m_vao = 0; m_vbo = 0; m_program = 0; m_uTime = -1;
}

/* ============================================================
 * Простая математика матриц (без зависимостей)
 * ============================================================ */
static void mat4Perspective(float fovY, float aspect, float nearZ, float farZ, float* m) {
    float f = 1.0f / std::tan(fovY * 0.5f);
    m[0]=f/aspect; m[1]=0; m[2]=0;  m[3]=0;
    m[4]=0;  m[5]=f; m[6]=0;  m[7]=0;
    m[8]=0;  m[9]=0; m[10]=(farZ+nearZ)/(nearZ-farZ); m[11]=-1;
    m[12]=0; m[13]=0; m[14]=2*farZ*nearZ/(nearZ-farZ); m[15]=0;
}

static void mat4LookAt(float ex,float ey,float ez, float cx,float cy,float cz, float* m) {
    float fx=cx-ex, fy=cy-ey, fz=cz-ez;
    float fl = std::sqrt(fx*fx+fy*fy+fz*fz);
    fx/=fl; fy/=fl; fz/=fl;
    /* right = up × forward */
    float ux=0, uy=1, uz=0;
    float rx = uy*fz - uz*fy;
    float ry = uz*fx - ux*fz;
    float rz = ux*fy - uy*fx;
    float rl = std::sqrt(rx*rx+ry*ry+rz*rz);
    if (rl > 0) { rx/=rl; ry/=rl; rz/=rl; }
    /* recompute up */
    ux = fy*rz - fz*ry;
    uy = fz*rx - fx*rz;
    uz = fx*ry - fy*rx;

    m[0]=rx; m[1]=ux; m[2]=-fx; m[3]=0;
    m[4]=ry; m[5]=uy; m[6]=-fy; m[7]=0;
    m[8]=rz; m[9]=uz; m[10]=-fz; m[11]=0;
    m[12]=-(rx*ex+ry*ey+rz*ez);
    m[13]=-(ux*ex+uy*ey+uz*ez);
    m[14]= fx*ex+fy*ey+fz*ez;
    m[15]=1;
}

void Render::draw(float time, const movement::Vec3& camPos, float yaw, float pitch) {
    glUseProgram(m_program);
    glUniform1f(m_uTime, time);

    /* View matrix */
    float dirX = std::cos(pitch) * std::sin(yaw);
    float dirY = std::sin(pitch);
    float dirZ = std::cos(pitch) * std::cos(yaw);

    float targetX = camPos.x + dirX;
    float targetY = camPos.y + dirY;
    float targetZ = camPos.z + dirZ;

    float viewMat[16];
    mat4LookAt(camPos.x, camPos.y, camPos.z,
               targetX, targetY, targetZ,
               viewMat);
    glUniformMatrix4fv(m_uView, 1, GL_FALSE, viewMat);

    /* Projection matrix */
    float projMat[16];
    mat4Perspective(70.0f * 3.14159f / 180.0f, 1280.0f / 720.0f, 0.1f, 100.0f, projMat);
    glUniformMatrix4fv(m_uProj, 1, GL_FALSE, projMat);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

} // namespace be::void_::core::render
