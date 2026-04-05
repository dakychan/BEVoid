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
 * BEVoid Project — drivers
 *
 * Абстрактный интерфейс рендера.
 * Каждая платформа реализует свой рендер на своём API:
 *   - Windows:  Direct3D 11
 *   - Linux:    OpenGL via GLX
 *   - Android:  OpenGL ES 3 via EGL
 */

#ifndef I_RENDERER_H
#define I_RENDERER_H

#include <cstdint>

namespace drivers {

class IRenderer {
public:
    virtual ~IRenderer() = default;

    /* Инициализировать рендер, привязать к окну */
    virtual bool init(void* nativeWindow, int32_t width, int32_t height) = 0;
    virtual void shutdown() = 0;

    /* Ресайз */
    virtual void resize(int32_t width, int32_t height) = 0;

    /* Очистка экрана */
    virtual void clear(float r, float g, float b, float a) = 0;

    /* Рендер кадра — здесь будет логика отрисовки */
    virtual void present() = 0;

    /* Загрузить шейдеры из исходного кода */
    virtual bool loadShaders(const char* vertexSrc, const char* fragmentSrc) = 0;

    /* Создать буфер вершин */
    virtual uint32_t createVertexBuffer(const float* data, int32_t count, int32_t stride) = 0;
    virtual void     destroyVertexBuffer(uint32_t id) = 0;

    /* Отрисовать буфер */
    virtual void drawBuffer(uint32_t bufferId, int32_t vertexCount) = 0;

    /* Обновить uniform */
    virtual void setUniformFloat(const char* name, float value) = 0;
};

} // namespace drivers

#endif // I_RENDERER_H
