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
 * Terrain shader — вершинный с освещением по нормалям,
 * фрагментный с цветом по высоте.
 */

#include "core/render/Render.h"
#include <iostream>
#include <fstream>
#include <string>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
    #include <android/log.h>
    #define LOG_TAG "BEVoid"
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
    #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)
#else
    #include <glad/glad.h>
    #define LOGI(...) std::printf(__VA_ARGS__)
    #define LOGE(...) std::fprintf(stderr, __VA_ARGS__)
#endif

namespace be::void_::core::render {

/* ============================================================
 * ШЕЙДЕРЫ — Android (GLES 3.0)
 * ============================================================ */
#if defined(BEVOID_PLATFORM_ANDROID)
static const char* A_VERT =
"#version 300 es\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"layout(location = 2) in vec3 aColor;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"out vec3 Color;\n"
"out float Height;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProj;\n"
"void main() {\n"
"    FragPos = aPos; Normal = aNormal; Color = aColor; Height = aPos.y;\n"
"    gl_Position = uProj * uView * vec4(aPos, 1.0);\n"
"}\n";

static const char* A_FRAG =
"#version 300 es\n"
"precision mediump float;\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"in vec3 Color;\n"
"in float Height;\n"
"out vec4 fragColor;\n"
"uniform vec3 uSunDir;\n"
"uniform vec3 uSunColor;\n"
"uniform vec3 uSkyColor;\n"
"uniform float uAmbient;\n"
"void main() {\n"
"    vec3 norm = normalize(Normal);\n"
"    float diff = max(dot(norm, uSunDir), 0.0);\n"
"    vec3 lighting = uAmbient * uSkyColor + diff * uSunColor;\n"
"    vec3 result = Color * lighting;\n"
"    float fog = clamp((Height - 50.0) / 150.0, 0.0, 1.0);\n"
"    result = mix(result, uSkyColor, fog * 0.3);\n"
"    fragColor = vec4(result, 1.0);\n"
"}\n";

/* ============================================================
 * ШЕЙДЕРЫ — Desktop (GL 3.3 core)
 * ============================================================ */
#else
static const char* D_VERT =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"layout(location = 1) in vec3 aNormal;\n"
"layout(location = 2) in vec3 aColor;\n"
"out vec3 FragPos;\n"
"out vec3 Normal;\n"
"out vec3 Color;\n"
"out float Height;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProj;\n"
"void main() {\n"
"    FragPos = aPos; Normal = aNormal; Color = aColor; Height = aPos.y;\n"
"    gl_Position = uProj * uView * vec4(aPos, 1.0);\n"
"}\n";

static const char* D_FRAG =
"#version 330 core\n"
"in vec3 FragPos;\n"
"in vec3 Normal;\n"
"in vec3 Color;\n"
"in float Height;\n"
"out vec4 fragColor;\n"
"uniform vec3 uSunDir;\n"
"uniform vec3 uSunColor;\n"
"uniform vec3 uSkyColor;\n"
"uniform float uAmbient;\n"
"void main() {\n"
"    vec3 norm = normalize(Normal);\n"
"    float diff = max(dot(norm, uSunDir), 0.0);\n"
"    vec3 lighting = uAmbient * uSkyColor + diff * uSunColor;\n"
"    vec3 result = Color * lighting;\n"
"    float fog = clamp((Height - 50.0) / 150.0, 0.0, 1.0);\n"
"    result = mix(result, uSkyColor, fog * 0.3);\n"
"    fragColor = vec4(result, 1.0);\n"
"}\n";
#endif

static std::string loadShaderFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    return std::string((std::istreambuf_iterator<char>(f)),
                        std::istreambuf_iterator<char>());
}

/* ============================================================
 * Математика матриц (COLUMN-MAJOR для OpenGL)
 * ============================================================ */
void Render::mat4Perspective(float fovY, float aspect, float nearZ, float farZ, float* m) {
    float f = 1.0f / std::tan(fovY * 0.5f);
    float rangeInv = 1.0f / (nearZ - farZ);
    /* column-major: m[col*4 + row] */
    m[0]=f/aspect; m[1]=0;   m[2]=0;              m[3]=0;
    m[4]=0;        m[5]=f;   m[6]=0;              m[7]=0;
    m[8]=0;        m[9]=0;   m[10]=(nearZ+farZ)*rangeInv; m[11]=-1;
    m[12]=0;       m[13]=0;  m[14]=2*nearZ*farZ*rangeInv; m[15]=0;
}

void Render::mat4LookAt(float ex,float ey,float ez, float cx,float cy,float cz, float* m) {
    float fx=cx-ex, fy=cy-ey, fz=cz-ez;
    float fl = std::sqrt(fx*fx+fy*fy+fz*fz);
    if (fl < 0.0001f) { fx=0; fy=0; fz=-1; fl=1; }
    fx/=fl; fy/=fl; fz/=fl;

    /* right = forward × up */
    float ux=0, uy=1, uz=0;
    float rx = fy*uz - fz*uy;
    float ry = fz*ux - fx*uz;
    float rz = fx*uy - fy*ux;
    float rl = std::sqrt(rx*rx+ry*ry+rz*rz);
    if (rl > 0.0001f) { rx/=rl; ry/=rl; rz/=rl; }

    /* recompute up = right × forward */
    ux = ry*fz - rz*fy;
    uy = rz*fx - rx*fz;
    uz = rx*fy - ry*fx;

    /* column-major view matrix */
    m[0]=rx;  m[1]=ux;  m[2]=-fx; m[3]=0;
    m[4]=ry;  m[5]=uy;  m[6]=-fy; m[7]=0;
    m[8]=rz;  m[9]=uz;  m[10]=-fz; m[11]=0;
    m[12]=-(rx*ex+ry*ey+rz*ez);
    m[13]=-(ux*ex+uy*ey+uz*ez);
    m[14]= fx*ex+fy*ey+fz*ez;
    m[15]=1;
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        LOGE("[Render] Shader error: %s\n", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

/* ============================================================ */
Render::Render() = default;
Render::~Render() = default;

bool Render::initShaders() {
#if defined(BEVOID_PLATFORM_ANDROID)
    const char* vs = A_VERT;
    const char* fs = A_FRAG;
#else
    std::string vsSrc = loadShaderFile("shaders/terrain.vert");
    std::string fsSrc = loadShaderFile("shaders/terrain.frag");
    std::string vsStr, fsStr;
    const char* vs;
    const char* fs;
    if (vsSrc.empty() || fsSrc.empty()) {
        LOGE("[Render] Shader files not found, using fallback\n");
        vs = D_VERT;
        fs = D_FRAG;
    } else {
        vsStr = vsSrc; fsStr = fsSrc;
        vs = vsStr.c_str();
        fs = fsStr.c_str();
    }
#endif

    GLuint vsSh = compileShader(GL_VERTEX_SHADER, vs);
    GLuint fsSh = compileShader(GL_FRAGMENT_SHADER, fs);
    if (!vsSh || !fsSh) return false;

    m_program = glCreateProgram();
    glAttachShader(m_program, vsSh);
    glAttachShader(m_program, fsSh);
    glLinkProgram(m_program);

    GLint ok;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_program, 512, nullptr, log);
        LOGE("[Render] Link error: %s\n", log);
        return false;
    }

    glDeleteShader(vsSh);
    glDeleteShader(fsSh);

    m_uTime   = glGetUniformLocation(m_program, "uTime");
    m_uView   = glGetUniformLocation(m_program, "uView");
    m_uProj    = glGetUniformLocation(m_program, "uProj");
    m_uCamPos  = glGetUniformLocation(m_program, "uCamPos");
    m_uSunDir   = glGetUniformLocation(m_program, "uSunDir");
    m_uSunColor = glGetUniformLocation(m_program, "uSunColor");
    m_uSkyColor = glGetUniformLocation(m_program, "uSkyColor");
    m_uAmbient  = glGetUniformLocation(m_program, "uAmbient");

    LOGI("[Render] Terrain shaders OK\n");
    return true;
}

bool Render::initSky() {
    bool ok = m_sky.init();
    if (ok) LOGI("[Render] Sky dome OK\n");
    else LOGE("[Render] Sky dome failed\n");
    return ok;
}

bool Render::initChunks() {
    return true; /* ChunkManager init внутри */
}

void Render::shutdown() {
    if (m_program) glDeleteProgram(m_program);
    m_program = 0;
}

void Render::updateChunks(float playerX, float playerZ, float dt) {
    m_chunkManager.update(playerX, playerZ, dt);
    m_cycles.update(dt);
}

void Render::drawSky(float time, float yaw, float pitch, int winWidth, int winHeight) {
    auto& st = m_cycles.getState();
    m_sky.setSkyColors(st.skyR, st.skyG, st.skyB, st.fogR, st.fogG, st.fogB);
    m_sky.setSunColor(st.sunColorR, st.sunColorG, st.sunColorB);

    float dirX = -std::cos(pitch) * std::sin(yaw);
    float dirY = std::sin(pitch);
    float dirZ = -std::cos(pitch) * std::cos(yaw);
    float viewMat[16];
    mat4LookAt(0, 0, 0, dirX, dirY, dirZ, viewMat); // центр в 0

    float aspect = winWidth > 0 && winHeight > 0 ? (float)winWidth / (float)winHeight : 1.777f;
    float projMat[16];
    mat4Perspective(70.0f * 3.14159f / 180.0f, aspect, 0.01f, 1000.0f, projMat);

    float sunEl = st.sunY; // elevation от sun direction
    m_sky.draw(time, viewMat, projMat, sunEl);
}

void Render::draw(float time, const Vec3& camPos, float yaw, float pitch, int winWidth, int winHeight) {
    auto& st = m_cycles.getState();

    // 1. Sky dome (без очистки глубины — sky на фоне)
    drawSky(time, yaw, pitch, winWidth, winHeight);

    // 2. Terrain
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(m_program);
    glUniform1f(m_uTime, time);

    glUniform3f(m_uSunDir, st.sunX, st.sunY, st.sunZ);
    glUniform3f(m_uSunColor, st.sunColorR, st.sunColorG, st.sunColorB);
    glUniform3f(m_uSkyColor, st.fogR, st.fogG, st.fogB);
    glUniform1f(m_uAmbient, st.ambientIntensity);

    float dirX = -std::cos(pitch) * std::sin(yaw);
    float dirY = std::sin(pitch);
    float dirZ = -std::cos(pitch) * std::cos(yaw);
    float viewMat[16];
    mat4LookAt(camPos.x, camPos.y, camPos.z,
               camPos.x + dirX, camPos.y + dirY, camPos.z + dirZ,
               viewMat);
    glUniformMatrix4fv(m_uView, 1, GL_FALSE, viewMat);

    float aspect = winWidth > 0 && winHeight > 0 ? (float)winWidth / (float)winHeight : 1.777f;
    float projMat[16];
    mat4Perspective(70.0f * 3.14159f / 180.0f, aspect, 0.01f, 1000.0f, projMat);
    glUniformMatrix4fv(m_uProj, 1, GL_FALSE, projMat);

    m_chunkManager.draw();

    drawCrosshair();
    drawHand();
}

void Render::drawCrosshair() {
    /* Кроссхаир — VBO, GL_LINES в NDC координатах */
    static GLuint crossVao = 0, crossVbo = 0;
    static bool crossInit = false;

    /* 4 линии: горизонталь (2 сегмента) + вертикаль (2 сегмента) = 8 вершин */
    static const float crossVerts[] = {
        /* X, Y */
        -0.025f,  0.0f,
        -0.008f,  0.0f,
         0.008f,  0.0f,
         0.025f,  0.0f,
         0.0f,   -0.025f,
         0.0f,   -0.008f,
         0.0f,    0.008f,
         0.0f,    0.025f,
    };

    if (!crossInit) {
        glGenVertexArrays(1, &crossVao);
        glGenBuffers(1, &crossVbo);
        glBindVertexArray(crossVao);
        glBindBuffer(GL_ARRAY_BUFFER, crossVbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(crossVerts), crossVerts, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindVertexArray(0);
        crossInit = true;
    }

    glDisable(GL_DEPTH_TEST);

    /* Простой шейдер для кроссхаира — белый цвет */
    static GLuint crossProg = 0;
    if (!crossProg) {
        const char* vs = "#version 330 core\nlayout(location=0) in vec2 p;void main(){gl_Position=vec4(p,0,1);}";
        const char* fs = "#version 330 core\nout vec4 c;void main(){c=vec4(1);}";
        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &vs, nullptr);
        glCompileShader(v);
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &fs, nullptr);
        glCompileShader(f);
        crossProg = glCreateProgram();
        glAttachShader(crossProg, v);
        glAttachShader(crossProg, f);
        glLinkProgram(crossProg);
        glDeleteShader(v);
        glDeleteShader(f);
    }

    glUseProgram(crossProg);
    glBindVertexArray(crossVao);
    glLineWidth(2.0f);
    glDrawArrays(GL_LINES, 0, 8);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void Render::drawHand() {
    /* Простая FPS-рука в NDC (правый нижний угол) */
    static GLuint vao = 0, vbo = 0, ebo = 0;
    static GLuint prog = 0;
    static bool init = false;

    /* Формат: X, Y, R, G, B */
    static const float verts[] = {
        /* Предплечье */
         0.72f, -0.95f,  0.55f, 0.35f, 0.20f,
         0.88f, -0.95f,  0.55f, 0.35f, 0.20f,
         0.88f, -0.60f,  0.55f, 0.35f, 0.20f,
         0.72f, -0.60f,  0.55f, 0.35f, 0.20f,
        /* Кисть */
         0.70f, -0.60f,  0.65f, 0.45f, 0.30f,
         0.90f, -0.60f,  0.65f, 0.45f, 0.30f,
         0.88f, -0.40f,  0.65f, 0.45f, 0.30f,
         0.72f, -0.40f,  0.65f, 0.45f, 0.30f,
        /* Пальцы (блок) */
         0.72f, -0.40f,  0.75f, 0.55f, 0.40f,
         0.76f, -0.40f,  0.75f, 0.55f, 0.40f,
         0.75f, -0.22f,  0.75f, 0.55f, 0.40f,
         0.71f, -0.22f,  0.75f, 0.55f, 0.40f,
    };
    static const uint32_t idx[] = {
         0,1,2,  2,3,0,   /* предплечье */
         4,5,6,  6,7,4,   /* кисть */
         8,9,10, 10,11,8  /* пальцы */
    };

    if (!init) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(2*sizeof(float)));
        glBindVertexArray(0);

        /* Простой шейдер для руки */
        const char* vs = "#version 330 core\nlayout(location=0) in vec2 p;layout(location=1) in vec3 c;out vec3 vc;void main(){gl_Position=vec4(p,0,1);vc=c;}";
        const char* fs = "#version 330 core\nin vec3 vc;out vec4 fc;void main(){fc=vec4(vc,1);}";
        GLuint v = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(v, 1, &vs, nullptr); glCompileShader(v);
        GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(f, 1, &fs, nullptr); glCompileShader(f);
        prog = glCreateProgram();
        glAttachShader(prog, v); glAttachShader(prog, f);
        glLinkProgram(prog);
        glDeleteShader(v); glDeleteShader(f);
        init = true;
    }

    glDisable(GL_DEPTH_TEST);
    glUseProgram(prog);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 18, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

} // namespace be::void_::core::render
