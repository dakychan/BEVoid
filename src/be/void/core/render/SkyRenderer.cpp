/*
 * ============================================================
 * BEVoid Project — Sky Dome Renderer
 * Полусфера с градиентом + облака (noise) + солнце
 * ============================================================
 */

#include "SkyRenderer.h"
#include "core/render/Render.h"
#include <cmath>
#include <vector>
#include <cstdint>
#include <fstream>
#include <string>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <android/log.h>
    #define SKY_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "BEVoid", __VA_ARGS__)
#else
    #include <cstdio>
    #define SKY_LOGE(...) std::fprintf(stderr, __VA_ARGS__)
#endif

namespace be::void_::core::render {

static std::string loadSkyShader(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

bool SkyRenderer::init() {
    std::string vsSrc = loadSkyShader("shaders/sky.vert");
    std::string fsSrc = loadSkyShader("shaders/sky.frag");
    
    if (vsSrc.empty()) {
        SKY_LOGE("[SkyRenderer] ERROR: shaders sky.vert не найден!\n");
        return false;
    }
    if (fsSrc.empty()) {
        SKY_LOGE("[SkyRenderer] ERROR: shaders sky.frag не найден!\n");
        return false;
    }
    
    SKY_LOGE("[SkyRenderer] Шейдеры загружены, компилю...\n");
    
    auto compile = [](GLuint type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char l[512]; glGetShaderInfoLog(s, 512, nullptr, l); 
            SKY_LOGE("[SkyRenderer] Compile error: %s\n", l);
            return 0; 
        }
        return s;
    };

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc.c_str());
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc.c_str());
    if (!vs || !fs) {
        SKY_LOGE("[SkyRenderer] ОШИБКА КОМПИЛЯЦИИ! vs=%d fs=%d\n", vs, fs);
        return false;
    }
    SKY_LOGE("[SkyRenderer] Шейдеры скомпилены OK\n");

    m_prog = glCreateProgram();
    glAttachShader(m_prog, vs); glAttachShader(m_prog, fs);
    glLinkProgram(m_prog);
    GLint linked;
    glGetProgramiv(m_prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[512];
        glGetProgramInfoLog(m_prog, 512, nullptr, log);
        SKY_LOGE("[SkyRenderer] ОШИБКА ЛИНКОВКИ: %s\n", log);
        return false;
    }
    SKY_LOGE("[SkyRenderer] Программа слинкована, m_prog=%d\n", m_prog);
    glDeleteShader(vs); glDeleteShader(fs);

    // Генерирую ПОЛНУЮ сферу (sky sphere) — чтобы закрыть горизонт
    const int RINGS = 16, SEGS = 32;
    struct V { float x, y, z; };
    std::vector<V> verts;
    std::vector<uint16_t> idx;

    // Полная сфера: от южного полюса до северного
    for (int r = 0; r <= RINGS; r++) {
        float phi = (float)r / RINGS * 3.14159f; // 0..PI (полная сфера!)
        float sinP = std::sin(phi), cosP = std::cos(phi);
        for (int s = 0; s <= SEGS; s++) {
            float theta = (float)s / SEGS * 6.28318f;
            verts.push_back({std::cos(theta) * sinP, cosP, std::sin(theta) * sinP});
        }
    }

    for (int r = 0; r < RINGS; r++) {
        for (int s = 0; s < SEGS; s++) {
            uint16_t a = r * (SEGS + 1) + s;
            uint16_t b = a + 1;
            uint16_t c = a + SEGS + 1;
            uint16_t d = c + 1;
            idx.push_back(a); idx.push_back(c); idx.push_back(b);
            idx.push_back(b); idx.push_back(c); idx.push_back(d);
        }
    }

    m_idxCount = (int)idx.size();

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)(verts.size() * sizeof(V)), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)(idx.size() * sizeof(uint16_t)), idx.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(V), nullptr);
    glBindVertexArray(0);

    /* ---- Инициализация солнца как отдельного объекта ---- */
    // Квад 4 вершины (x,y,u,v) + 6 индексов
    struct SunVert { float x, y, u, v; };
    static const SunVert sunVerts[4] = {
        {-1, -1, 0, 0},
        { 1, -1, 1, 0},
        { 1,  1, 1, 1},
        {-1,  1, 0, 1},
    };
    static const uint16_t sunIdx[6] = {0,1,2, 0,2,3};

    glGenVertexArrays(1, &m_sunVao);
    glGenBuffers(1, &m_sunVbo);
    glBindVertexArray(m_sunVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_sunVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sunVerts), sunVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(SunVert), nullptr);
    glBindVertexArray(0);

    // Шейдер для солнца
    const char* sunVS =
        "#version 330 core\n"
        "layout(location=0) in vec4 aPosUV;\n"
        "out vec2 vUV;\n"
        "uniform mat4 uVP;\n"
        "uniform vec3 uCenter;   // мировая позиция солнца\n"
        "uniform vec3 uRight;    // правый вектор камеры\n"
        "uniform vec3 uUp;       // верхний вектор камеры\n"
        "uniform float uSize;    // размер диска\n"
        "void main(){\n"
        "    vUV = aPosUV.zw;\n"
        "    vec3 worldPos = uCenter + aPosUV.x * uRight * uSize + aPosUV.y * uUp * uSize;\n"
        "    gl_Position = uVP * vec4(worldPos, 1.0);\n"
        "}\n";
    const char* sunFS =
        "#version 330 core\n"
        "in vec2 vUV;\n"
        "out vec4 fragColor;\n"
        "uniform vec3 uColor;\n"
        "void main(){\n"
        "    vec2 c = vUV - 0.5;\n"
        "    float d = length(c);\n"
        "    // Мягкий диск с glow\n"
        "    float core = 1.0 - smoothstep(0.35, 0.5, d);\n"
        "    float glow = pow(1.0 - d, 3.0) * 0.6;\n"
        "    float alpha = max(core, glow);\n"
        "    fragColor = vec4(uColor * (1.0 + glow), alpha);\n"
        "}\n";

    auto compileShader = [](GLuint type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char l[512]; glGetShaderInfoLog(s, 512, nullptr, l);
            SKY_LOGE("[SkyRenderer] Sun shader compile error: %s\n", l);
            return 0;
        }
        return s;
    };

    GLuint sunVS_sh = compileShader(GL_VERTEX_SHADER, sunVS);
    GLuint sunFS_sh = compileShader(GL_FRAGMENT_SHADER, sunFS);
    if (sunVS_sh && sunFS_sh) {
        m_sunProg = glCreateProgram();
        glAttachShader(m_sunProg, sunVS_sh);
        glAttachShader(m_sunProg, sunFS_sh);
        glLinkProgram(m_sunProg);
        GLint sunLinked;
        glGetProgramiv(m_sunProg, GL_LINK_STATUS, &sunLinked);
        if (!sunLinked) {
            char log[512];
            glGetProgramInfoLog(m_sunProg, 512, nullptr, log);
            SKY_LOGE("[SkyRenderer] Sun shader link error: %s\n", log);
            m_sunProg = 0;
        }
        glDeleteShader(sunVS_sh);
        glDeleteShader(sunFS_sh);
    }

    SKY_LOGE("[SkyRenderer] init УСПЕХ! VAO=%d VBO=%d idxCount=%d sunProg=%d\n", m_vao, m_vbo, m_idxCount, m_sunProg);
    return true;
}

void SkyRenderer::draw(float time, const float* viewMat, const float* projMat, float sunElevation) {
    if (m_prog == 0 || m_vao == 0) {
        SKY_LOGE("[SkyRenderer] draw ПЫТАЮСЬ рисовать но m_prog=%d m_vao=%d !!!\n", m_prog, m_vao);
        return;
    }
    
    glUseProgram(m_prog);
    glUniform1f(glGetUniformLocation(m_prog, "uTime"), time);
    glUniform3f(glGetUniformLocation(m_prog, "uTopColor"), m_topR, m_topG, m_topB);
    glUniform3f(glGetUniformLocation(m_prog, "uHorizonColor"), m_horizonR, m_horizonG, m_horizonB);
    glUniform3f(glGetUniformLocation(m_prog, "uSunColor"), m_sunR, m_sunG, m_sunB);
    glUniform3f(glGetUniformLocation(m_prog, "uSunDir"), m_sunDirX, m_sunDirY, m_sunDirZ);
    glUniform3f(glGetUniformLocation(m_prog, "uMoonColor"), m_moonR, m_moonG, m_moonB);
    glUniform3f(glGetUniformLocation(m_prog, "uMoonDir"), m_moonDirX, m_moonDirY, m_moonDirZ);
    glUniform1f(glGetUniformLocation(m_prog, "uSunElevation"), sunElevation);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uView"), 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uProj"), 1, GL_FALSE, projMat);

    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_idxCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        SKY_LOGE("[SkyRenderer] OpenGL ошибка после draw: 0x%X\n", err);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

void SkyRenderer::drawSun(const float* projMat, const float* viewMat) {
    if (m_sunProg == 0 || m_sunVao == 0) return;
    if (m_sunDirY < -0.1f) return;

    // Умножаем proj * view (с обнулённой трансляцией) для billboard
    float vNoTrans[16];
    for (int i = 0; i < 16; i++) vNoTrans[i] = viewMat[i];
    vNoTrans[12] = 0; vNoTrans[13] = 0; vNoTrans[14] = 0;

    float vp[16];
    for (int c = 0; c < 4; c++)
        for (int r = 0; r < 4; r++)
            vp[c * 4 + r] = projMat[r] * vNoTrans[c*4] + projMat[4+r] * vNoTrans[c*4+1] +
                            projMat[8+r] * vNoTrans[c*4+2] + projMat[12+r] * vNoTrans[c*4+3];

    // Позиция солнца в мировом пространстве
    float sunDist = 400.0f;
    float sunX = m_sunDirX * sunDist;
    float sunY = m_sunDirY * sunDist;
    float sunZ = m_sunDirZ * sunDist;

    // Right и Up из view матрицы
    float rightX = viewMat[0], rightY = viewMat[1], rightZ = viewMat[2];
    float upX = viewMat[4], upY = viewMat[5], upZ = viewMat[6];

    // Размер солнца — побольше чтобы было видно
    float sunSize = 25.0f;

    glUseProgram(m_sunProg);
    glUniformMatrix4fv(glGetUniformLocation(m_sunProg, "uVP"), 1, GL_FALSE, vp);
    glUniform3f(glGetUniformLocation(m_sunProg, "uCenter"), sunX, sunY, sunZ);
    glUniform3f(glGetUniformLocation(m_sunProg, "uRight"), rightX, rightY, rightZ);
    glUniform3f(glGetUniformLocation(m_sunProg, "uUp"), upX, upY, upZ);
    glUniform3f(glGetUniformLocation(m_sunProg, "uColor"), m_sunR, m_sunG, m_sunB);
    glUniform1f(glGetUniformLocation(m_sunProg, "uSize"), sunSize);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE); // аддитивное блендинг для свечения

    glBindVertexArray(m_sunVao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void SkyRenderer::shutdown() {
    if (m_prog) glDeleteProgram(m_prog);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_sunProg) glDeleteProgram(m_sunProg);
    if (m_sunVao) glDeleteVertexArrays(1, &m_sunVao);
    if (m_sunVbo) glDeleteBuffers(1, &m_sunVbo);
    m_prog = 0; m_vao = 0; m_vbo = 0; m_ebo = 0;
    m_sunProg = 0; m_sunVao = 0; m_sunVbo = 0;
}

} // namespace
