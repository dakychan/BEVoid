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
 * Terrain renderer — чанки с шумом, 3D камера.
 */

#ifndef BEVOID_RENDER_H
#define BEVOID_RENDER_H

#include "core/movement/Movement.h"
#include "core/render/chunk/ChunkManager.h"
#include "physics/Cycles.h"
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

    bool initShaders();
    bool initChunks();
    void shutdown();

    void draw(float time, const movement::Vec3& camPos, float yaw, float pitch);
    void updateChunks(float playerX, float playerZ, float dt);

    chunk::ChunkManager& getChunkManager() { return m_chunkManager; }

    /* Terrain shader */
    GLuint m_program  = 0;
    GLint  m_uTime    = -1;
    GLint  m_uView    = -1;
    GLint  m_uProj    = -1;
    GLint  m_uCamPos  = -1;
    GLint  m_uSunDir   = -1;
    GLint  m_uSunColor = -1;
    GLint  m_uSkyColor = -1;
    GLint  m_uAmbient  = -1;

    chunk::ChunkManager m_chunkManager;
    physics::Cycles     m_cycles;

    /* Кроссхаир */
    void drawCrosshair();
    void drawHand();

    /* Математика матриц (column-major для OpenGL) */
    static void mat4Perspective(float fovY, float aspect, float nearZ, float farZ, float* out);
    static void mat4LookAt(float ex, float ey, float ez, float cx, float cy, float cz, float* out);
};

} // namespace be::void_::core::render

#endif // BEVOID_RENDER_H
