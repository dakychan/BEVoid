/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#ifndef BEVOID_RENDER_H
#define BEVOID_RENDER_H

#include "core/movement/Movement.h"
#include "core/render/world/ChunkManager.h"
#include "core/render/SkyRenderer.h"
#include "physics/Cycles.h"
#include "Vec3.h"
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
    bool initSky();
    bool initChunks();
    void shutdown();

    void draw(float time, const Vec3& camPos, float yaw, float pitch, int winWidth, int winHeight);
    void updateChunks(float playerX, float playerZ, float dt);

    world::ChunkManager& getChunkManager() { return m_chunkManager; }

    void drawCrosshair();
    void drawHand();

    static void mat4Perspective(float fovY, float aspect, float nearZ, float farZ, float* out);
    static void mat4LookAt(float ex, float ey, float ez, float cx, float cy, float cz, float* out);

private:
    GLuint m_program  = 0;
    GLint  m_uTime    = -1;
    GLint  m_uView    = -1;
    GLint  m_uProj    = -1;
    GLint  m_uCamPos  = -1;
    GLint  m_uSunDir   = -1;
    GLint  m_uSunColor = -1;
    GLint  m_uSkyColor = -1;
    GLint  m_uHorizonColor = -1;
    GLint  m_uAmbient  = -1;

    world::ChunkManager m_chunkManager;
    SkyRenderer         m_sky;
    bool                m_skyOk = false;
    physics::Cycles     m_cycles;

    GLuint m_crossVao = 0, m_crossVbo = 0;
    GLuint m_handVao = 0, m_handVbo = 0, m_handEbo = 0;
};

} // namespace be::void_::core::render

#endif // BEVOID_RENDER_H
