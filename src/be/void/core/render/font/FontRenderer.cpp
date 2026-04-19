#include "core/render/font/FontRenderer.h"
#include <iostream>
#include <cstring>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render::font {

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << "[FontRenderer] Shader error: " << log << "\n";
        glDeleteShader(s);
        return 0;
    }
    return s;
}

FontRenderer::FontRenderer() {
    m_vertData = new QuadVert[MAX_QUADS * 6];
}

FontRenderer::~FontRenderer() {
    shutdown();
    delete[] m_vertData;
}

bool FontRenderer::init() {
    if (m_initialized) return true;
    ensureShader();
    if (!m_program) return false;

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_QUADS * 6 * sizeof(QuadVert), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)8);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(QuadVert), (void*)16);

    glBindVertexArray(0);

    m_initialized = true;
    return true;
}

void FontRenderer::shutdown() {
    if (m_vao) { glDeleteVertexArrays(1, &m_vao); m_vao = 0; }
    if (m_vbo) { glDeleteBuffers(1, &m_vbo); m_vbo = 0; }
    if (m_program) { glDeleteProgram(m_program); m_program = 0; }
    m_fonts.clear();
    m_initialized = false;
}

void FontRenderer::ensureShader() {
    if (m_program) return;

    const char* vs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
#else
        "#version 330 core\n"
#endif
        "layout(location = 0) in vec2 aPos;\n"
        "layout(location = 1) in vec2 aUV;\n"
        "layout(location = 2) in vec4 aColor;\n"
        "out vec2 vUV;\n"
        "out vec4 vColor;\n"
        "void main() {\n"
        "    gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "    vUV = aUV;\n"
        "    vColor = aColor;\n"
        "}\n";

    const char* fs =
#if defined(BEVOID_PLATFORM_ANDROID)
        "#version 300 es\n"
        "precision mediump float;\n"
#else
        "#version 330 core\n"
#endif
        "in vec2 vUV;\n"
        "in vec4 vColor;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D uTexture;\n"
        "uniform float uPxRange;\n"
        "\n"
        "float median(float r, float g, float b) {\n"
        "    return max(min(r, g), min(max(r, g), b));\n"
        "}\n"
        "\n"
        "void main() {\n"
        "    vec3 msdf = texture(uTexture, vUV).rgb;\n"
        "    float sd = median(msdf.r, msdf.g, msdf.b) - 0.5;\n"
        "    float screenPxDist = fwidth(sd) * 0.5;\n"
        "    float opacity = clamp((sd / screenPxDist) + 0.5, 0.0, 1.0);\n"
        "    fragColor = vec4(vColor.rgb, vColor.a * opacity);\n"
        "}\n";

    GLuint v = compileShader(GL_VERTEX_SHADER, vs);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
    if (!v || !f) return;

    m_program = glCreateProgram();
    glAttachShader(m_program, v);
    glAttachShader(m_program, f);
    glLinkProgram(m_program);

    GLint ok;
    glGetProgramiv(m_program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(m_program, 512, nullptr, log);
        std::cerr << "[FontRenderer] Link error: " << log << "\n";
        glDeleteProgram(m_program);
        m_program = 0;
    }

    glDeleteShader(v);
    glDeleteShader(f);

    if (m_program) {
        m_uTexture  = glGetUniformLocation(m_program, "uTexture");
        m_uPxRange  = glGetUniformLocation(m_program, "uPxRange");
    }
}

void FontRenderer::loadFont(const std::string& name, const std::string& baseName) {
    auto atlas = std::make_unique<FontAtlas>();
    std::string jsonPath = "assets/fonts/" + baseName + ".json";
    std::string pngPath  = "assets/fonts/" + baseName + ".png";

    if (atlas->load(jsonPath, pngPath)) {
        atlas->uploadTexture();
        m_fonts[name] = std::move(atlas);
    } else {
        std::cerr << "[FontRenderer] Failed to load font: " << name << "\n";
    }
}

FontAtlas* FontRenderer::getFont(const std::string& name) {
    auto it = m_fonts.find(name);
    return it != m_fonts.end() ? it->second.get() : nullptr;
}

float FontRenderer::getTextWidth(const std::string& fontName, const std::string& text, float scale) {
    FontAtlas* atlas = getFont(fontName);
    if (!atlas) return 0;
    float pxScale = scale / atlas->getFontSize();
    float w = 0;
    for (char c : text) {
        const Glyph* g = atlas->getGlyph((unsigned char)c);
        if (g) w += g->xAdvance * pxScale;
        else   w += atlas->getFontSize() * 0.25f * pxScale;
    }
    return w;
}

void FontRenderer::buildQuads(FontAtlas& atlas, const std::string& text,
                               float x, float y, float scale,
                               float r, float g, float b, float a) {
    float cursorX = x;
    float cursorY = y;

    for (size_t i = 0; i < text.size(); i++) {
        int cp = (unsigned char)text[i];

        if (cp == '\n') {
            cursorX = x;
            cursorY -= atlas.getLineHeight() * scale;
            continue;
        }

        const Glyph* gl = atlas.getGlyph(cp);
        if (!gl) {
            cursorX += atlas.getFontSize() * 0.25f * scale;
            continue;
        }

        if (gl->width > 0 && gl->height > 0) {
            float x0 = cursorX + gl->xOffset * scale;
            float y1 = cursorY - gl->yOffset * scale;
            float x1 = x0 + gl->width * scale;
            float y0 = y1 - gl->height * scale;

            if (m_vertCount + 6 <= MAX_QUADS * 6) {
                QuadVert* v = m_vertData + m_vertCount;

                v[0] = {x0, y0, gl->u0, gl->v0, r, g, b, a};
                v[1] = {x1, y0, gl->u1, gl->v0, r, g, b, a};
                v[2] = {x1, y1, gl->u1, gl->v1, r, g, b, a};
                v[3] = {x0, y0, gl->u0, gl->v0, r, g, b, a};
                v[4] = {x1, y1, gl->u1, gl->v1, r, g, b, a};
                v[5] = {x0, y1, gl->u0, gl->v1, r, g, b, a};

                m_vertCount += 6;
            }
        }

        cursorX += gl->xAdvance * scale;
    }
}

void FontRenderer::drawText(const std::string& fontName, const std::string& text,
                             float x, float y, float scale,
                             float r, float g, float b, float a) {
    FontAtlas* atlas = getFont(fontName);
    if (!atlas || !m_program) {
        std::cerr << "[FontRenderer] drawText: no atlas=" << (atlas ? "Y" : "N") << " prog=" << m_program << "\n";
        return;
    }

    float pxScale = scale / atlas->getFontSize();

    m_vertCount = 0;
    buildQuads(*atlas, text, x, y, pxScale, r, g, b, a);
    if (m_vertCount == 0) {
        std::cerr << "[FontRenderer] drawText: 0 verts for '" << text << "' pxScale=" << pxScale << " fontSize=" << atlas->getFontSize() << "\n";
        return;
    }
    static bool once = false;
    if (!once) {
        once = true;
        std::cerr << "[FontRenderer] drawText OK: '" << text << "' verts=" << m_vertCount << " pxScale=" << pxScale << " tex=" << atlas->getTexture() << "\n";
    }

    glUseProgram(m_program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlas->getTexture());
    glUniform1i(m_uTexture, 0);
    if (m_uPxRange >= 0) glUniform1f(m_uPxRange, atlas->getDistanceRange());

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_vertCount * sizeof(QuadVert), m_vertData);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, m_vertCount);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glUseProgram(0);
}

} // namespace
