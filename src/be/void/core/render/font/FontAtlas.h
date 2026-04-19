#ifndef BEVOID_FONT_ATLAS_H
#define BEVOID_FONT_ATLAS_H

#include "Glyph.h"
#include <string>
#include <unordered_map>

namespace be::void_::core::render::font {

class FontAtlas {
public:
    FontAtlas() = default;

    bool load(const std::string& jsonPath, const std::string& pngPath);

    const Glyph* getGlyph(int codePoint) const;
    bool hasGlyph(int codePoint) const;

    unsigned int getTexture()     const { return m_texture; }
    float  getFontSize()    const { return m_fontSize; }
    float  getLineHeight()  const { return m_lineHeight; }
    float  getAtlasWidth()  const { return m_atlasWidth; }
    float  getAtlasHeight() const { return m_atlasHeight; }
    float  getDistanceRange() const { return m_distanceRange; }
    float  getAscender()    const { return m_ascender; }
    bool   isLoaded()       const { return m_loaded; }
    int    getGlyphCount()  const { return (int)m_glyphs.size(); }

    void uploadTexture();

private:
    std::unordered_map<int, Glyph> m_glyphs;
    unsigned int m_texture = 0;
    float  m_atlasWidth = 512;
    float  m_atlasHeight = 512;
    float  m_fontSize = 32;
    float  m_lineHeight = 40;
    float  m_distanceRange = 4;
    float  m_ascender = 0.95f;
    bool   m_yOriginBottom = false;
    bool   m_loaded = false;
    bool   m_textureUploaded = false;

    int    m_pngWidth = 0;
    int    m_pngHeight = 0;
    unsigned char* m_pngData = nullptr;
};

} // namespace

#endif
