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
 * Рендерер — отрисовка геометрии, шейдеры, блоки.
 * Сейчас: базовый VAO/VBO + треугольник.
 * В будущем: чанки, блоки, меши, текстуры.
 */

#ifndef BEVOID_RENDER_H
#define BEVOID_RENDER_H

#include "core/movement/Movement.h"
#include <cstdint>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render {

class Render {
public:
    Render();
    ~Render();

    /* Инициализация шейдеров и геометрии */
    bool initShaders();
    bool initGeometry();
    void shutdown();

    /* Отрисовка кадра */
    void draw(float time, const movement::Vec3& camPos, float yaw, float pitch);

    /* Доступ к GL объектам */
    GLuint getProgram() const { return m_program; }
    GLint  getUTime()   const { return m_uTime;   }

private:
    GLuint m_vao     = 0;
    GLuint m_vbo     = 0;
    GLuint m_ebo     = 0;
    GLuint m_program = 0;
    GLint  m_uTime   = -1;
    GLint  m_uView   = -1;
    GLint  m_uProj   = -1;
    int    m_indexCount = 0;
};

} // namespace be::void_::core::render

#endif // BEVOID_RENDER_H
