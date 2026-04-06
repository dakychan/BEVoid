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

namespace be::void_::core::render {

#if defined(BEVOID_PLATFORM_ANDROID)
static const char* SKY_VERT =
"#version 300 es\n"
"layout(location = 0) in vec3 aPos;\n"
"out vec3 vDir;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProj;\n"
"void main() {\n"
"    vDir = aPos;\n"
"    mat4 v = uView;\n"
"    v[3][0] = 0.0; v[3][1] = 0.0; v[3][2] = 0.0;\n"
"    gl_Position = (uProj * v * vec4(aPos * 500.0, 1.0)).xyww;\n"
"}\n";

static const char* SKY_FRAG =
"#version 300 es\n"
"precision mediump float;\n"
"in vec3 vDir;\n"
"out vec4 fragColor;\n"
"uniform float uTime;\n"
"uniform vec3 uTopColor;\n"
"uniform vec3 uHorizonColor;\n"
"uniform vec3 uSunColor;\n"
"uniform float uSunElevation;\n"

// Simple hash noise
"float hash(vec3 p){\n"
"    p=fract(p*vec3(.1031,.1030,.0973));\n"
"    p+=dot(p,p+33.33);\n"
"    return fract((p.x+p.y)*p.z);\n"
"}\n"

"float noise(vec3 p){\n"
"    vec3 i=floor(p),f=fract(p);\n"
"    f=f*f*(3.-2.*f);\n"
"    return mix(mix(mix(hash(i),hash(i+vec3(1,0,0)),f.x),\n"
"       mix(hash(i+vec3(0,1,0)),hash(i+vec3(1,1,0)),f.x),f.y),\n"
"       mix(mix(hash(i+vec3(0,0,1)),hash(i+vec3(1,0,1)),f.x),\n"
"       mix(hash(i+vec3(0,1,1)),hash(i+vec3(1,1,1)),f.x),f.y),f.z);\n"
"}\n"

"float fbm(vec3 p){\n"
"    float v=0.,a=.5;\n"
"    mat3 m=mat3(1.,0.,0.,0.,1.,0.,0.,0.,1.);\n"
"    for(int i=0;i<5;i++){\n"
"        v+=a*noise(p);\n"
"        p=p*2.01;\n"
"        a*=.5;\n"
"    }\n"
"    return v;\n"
"}\n"

"void main(){\n"
"    vec3 dir=normalize(vDir);\n"
"    float h=dir.y;\n"

// Яркий градиент неба
"    float t=pow(max(h,0.),0.45);\n"
"    vec3 sky=mix(uHorizonColor,uTopColor,t);\n"

// Солнце
"    vec3 sd=normalize(vec3(.5,max(uSunElevation,.15),.3));\n"
"    float sdot=max(dot(dir,sd),0.);\n"
"    sky+=uSunColor*(pow(sdot,300.)*5.+pow(sdot,8.)*0.35);\n"

// Облака — яркие и заметные
"    if(h>0.01){\n"
"        vec3 cp=vec3(dir.xz*5.,uTime*.025);\n"
"        float c=fbm(cp);\n"
"        c=smoothstep(.38,.68,c);\n"
"        float fade=smoothstep(.01,.12,h);\n"
"        vec3 cc=vec3(.92,.93,.96)*(0.55+0.45*sdot);\n"
"        sky=mix(sky,cc,c*fade*.85);\n"
"    }\n"

"    fragColor=vec4(sky,1.);\n"
"}\n";
#else
static const char* SKY_VERT =
"#version 330 core\n"
"layout(location = 0) in vec3 aPos;\n"
"out vec3 vDir;\n"
"uniform mat4 uView;\n"
"uniform mat4 uProj;\n"
"void main() {\n"
"    vDir = aPos;\n"
"    mat4 v = uView;\n"
"    v[3][0] = 0.0; v[3][1] = 0.0; v[3][2] = 0.0;\n"
"    gl_Position = (uProj * v * vec4(aPos * 500.0, 1.0)).xyww;\n"
"}\n";

static const char* SKY_FRAG =
"#version 330 core\n"
"in vec3 vDir;\n"
"out vec4 fragColor;\n"
"uniform float uTime;\n"
"uniform vec3 uTopColor;\n"
"uniform vec3 uHorizonColor;\n"
"uniform vec3 uSunColor;\n"
"uniform float uSunElevation;\n"

"vec3 mod289(vec3 x){return x-floor(x*(1./289.))*289.;}\n"
"vec4 mod289(vec4 x){return x-floor(x*(1./289.))*289.;}\n"
"vec4 perm(vec4 x){return mod289(((x*34.)+1.)*x);}\n"
"float noise(vec3 p){\n"
"    p=mod289(p);vec4 a=floor(p.yzxw);vec4 b=fract(p.yzxw);\n"
"    b=b*b*(3.-2.*b);vec4 i=perm(a+perm(a.x+b.x));\n"
"    vec4 c=fract(i);vec4 o1=fract(i*(1./41.));\n"
"    vec4 o2=fract(o1*13.);vec4 o3=mix(o1,o2,b.x);\n"
"    vec4 o4=mix(o3,o3+vec4(.25),b.y);\n"
"    return mix(mix(o4.x,o4.z,b.z),mix(o4.y,o4.w,b.z),b.w);\n"
"}\n"
"float fbm(vec3 p){\n"
"    float v=0.,a=.5;for(int i=0;i<5;i++){v+=a*noise(p);p*=2.01;a*=.5;}return v;\n"
"}\n"
"void main(){\n"
"    vec3 dir=normalize(vDir);float h=dir.y;\n"
"    float t=pow(max(h,0.),0.45);\n"
"    vec3 sky=mix(uHorizonColor,uTopColor,t);\n"
"    vec3 sd=normalize(vec3(.5,max(uSunElevation,.15),.3));\n"
"    float sdot=max(dot(dir,sd),0.);\n"
"    sky+=uSunColor*(pow(sdot,300.)*5.+pow(sdot,8.)*0.35);\n"
"    if(h>0.01){\n"
"        vec3 cp=vec3(dir.xz*5.,uTime*.025);\n"
"        float c=fbm(cp);\n"
"        c=smoothstep(.38,.68,c);\n"
"        sky=mix(sky,vec3(.92,.93,.96)*(0.55+0.45*sdot),c*smoothstep(.01,.12,h)*.85);\n"
"    }\n"
"    fragColor=vec4(sky,1.);\n"
"}\n";
#endif

bool SkyRenderer::init() {
    auto compile = [](GLuint type, const char* src) -> GLuint {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char l[512]; glGetShaderInfoLog(s, 512, nullptr, l); return 0; }
        return s;
    };

    GLuint vs = compile(GL_VERTEX_SHADER, SKY_VERT);
    GLuint fs = compile(GL_FRAGMENT_SHADER, SKY_FRAG);
    if (!vs || !fs) return false;

    m_prog = glCreateProgram();
    glAttachShader(m_prog, vs); glAttachShader(m_prog, fs);
    glLinkProgram(m_prog);
    glDeleteShader(vs); glDeleteShader(fs);

    // Генерирую полусферу (sky dome)
    const int RINGS = 16, SEGS = 32;
    struct V { float x, y, z; };
    std::vector<V> verts;
    std::vector<uint16_t> idx;

    // Верхний колпак
    for (int r = 0; r <= RINGS; r++) {
        float phi = (float)r / RINGS * 1.570796f; // 0..PI/2
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

    return true;
}

void SkyRenderer::draw(float time, const float* viewMat, const float* projMat, float sunElevation) {
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_prog);
    glUniform1f(glGetUniformLocation(m_prog, "uTime"), time);
    glUniform3f(glGetUniformLocation(m_prog, "uTopColor"), m_topR, m_topG, m_topB);
    glUniform3f(glGetUniformLocation(m_prog, "uHorizonColor"), m_horizonR, m_horizonG, m_horizonB);
    glUniform3f(glGetUniformLocation(m_prog, "uSunColor"), m_sunR, m_sunG, m_sunB);
    glUniform1f(glGetUniformLocation(m_prog, "uSunElevation"), sunElevation);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uView"), 1, GL_FALSE, viewMat);
    glUniformMatrix4fv(glGetUniformLocation(m_prog, "uProj"), 1, GL_FALSE, projMat);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_idxCount, GL_UNSIGNED_SHORT, nullptr);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
}

void SkyRenderer::shutdown() {
    if (m_prog) glDeleteProgram(m_prog);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    m_prog = 0; m_vao = 0; m_vbo = 0; m_ebo = 0;
}

} // namespace
