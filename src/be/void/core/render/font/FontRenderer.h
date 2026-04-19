#ifndef BEVOID_FONT_RENDERER_H
#define BEVOID_FONT_RENDERER_H

#include "FontAtlas.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace be::void_::core::render::font {

class FontRenderer {
public:
    FontRenderer();
    ~FontRenderer();

    bool init();
    void shutdown();

    void loadFont(const std::string& name, const std::string& baseName);
    FontAtlas* getFont(const std::string& name);

    void drawText(const std::string& fontName, const std::string& text,
                  float x, float y, float scale,
                  float r, float g, float b, float a = 1.0f);

    float getTextWidth(const std::string& fontName, const std::string& text, float scale);

private:
    void ensureShader();
    void buildQuads(FontAtlas& atlas, const std::string& text,
                    float x, float y, float scale,
                    float r, float g, float b, float a);

    std::unordered_map<std::string, std::unique_ptr<FontAtlas>> m_fonts;

    unsigned int m_program = 0;
    int  m_uTexture = -1;
    int  m_uPxRange = -1;

    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;

    static const int MAX_QUADS = 4096;

    struct QuadVert {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    QuadVert* m_vertData = nullptr;
    int m_vertCount = 0;
    bool m_initialized = false;
};

} // namespace

#endif
