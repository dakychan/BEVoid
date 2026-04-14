/*
 * ============================================================
 * BEVoid Project — Sky Dome Renderer
 * ============================================================
 */

#ifndef BEVOID_SKY_RENDERER_H
#define BEVOID_SKY_RENDERER_H

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render {

class SkyRenderer {
public:
    bool init();
    void draw(float time, const float* viewMat, const float* projMat, float sunElevation);
    void drawWithCamPos(float time, const float* viewMat, const float* projMat, float sunElevation, float camX, float camY, float camZ);
    void drawSunBillboard(const float* projMat, const float* viewMat, float camX, float camY, float camZ);
    void shutdown();

    void setSunColor(float r, float g, float b) { m_sunR = r; m_sunG = g; m_sunB = b; }
    void setSunDirection(float x, float y, float z) { m_sunDirX = x; m_sunDirY = y; m_sunDirZ = z; }
    void setMoonColor(float r, float g, float b) { m_moonR = r; m_moonG = g; m_moonB = b; }
    void setMoonDirection(float x, float y, float z) { m_moonDirX = x; m_moonDirY = y; m_moonDirZ = z; }
    void setSkyColors(float topR, float topG, float topB,
                      float horizonR, float horizonG, float horizonB) {
        m_topR = topR; m_topG = topG; m_topB = topB;
        m_horizonR = horizonR; m_horizonG = horizonG; m_horizonB = horizonB;
    }

private:
    GLuint m_prog  = 0;
    GLuint m_vao   = 0;
    GLuint m_vbo   = 0;
    GLuint m_ebo   = 0;
    int    m_idxCount = 0;

    GLuint m_sunProg = 0;
    GLuint m_sunVao  = 0;
    GLuint m_sunVbo  = 0;

    float m_sunR = 1.0f, m_sunG = 0.9f, m_sunB = 0.6f;
    float m_sunDirX = 0.5f, m_sunDirY = 0.5f, m_sunDirZ = 0.3f;
    float m_moonR = 0.8f, m_moonG = 0.82f, m_moonB = 0.95f;
    float m_moonDirX = 0.0f, m_moonDirY = 0.5f, m_moonDirZ = 0.0f;
    float m_topR = 0.2f, m_topG = 0.4f, m_topB = 0.8f;
    float m_horizonR = 0.6f, m_horizonG = 0.75f, m_horizonB = 0.9f;
};

} // namespace
#endif
