#include "core/render/font/FontAtlas.h"
#include <stb_image.h>
#include <fstream>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <vector>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
#endif

namespace be::void_::core::render::font {

struct JVal;
struct JObj {
    std::vector<std::pair<std::string,JVal>> entries;
};
struct JArr {
    std::vector<JVal> items;
};
struct JVal {
    enum Type { NUL, NUM, STR, OBJ, ARR } type = NUL;
    double num = 0;
    std::string str;
    JObj obj;
    JArr arr;
};

static void skipWs(const char*& p) {
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') p++;
}

static std::string parseStr(const char*& p) {
    skipWs(p);
    if (*p != '"') return "";
    p++;
    std::string r;
    while (*p && *p != '"') {
        if (*p == '\\') { p++; if (*p == 'n') r += '\n'; else if (*p == 't') r += '\t'; else if (*p) r += *p; }
        else r += *p;
        p++;
    }
    if (*p == '"') p++;
    return r;
}

static JVal parseVal(const char*& p);

static JObj parseObj(const char*& p) {
    JObj o;
    skipWs(p);
    if (*p != '{') return o;
    p++;
    skipWs(p);
    while (*p && *p != '}') {
        std::string key = parseStr(p);
        skipWs(p);
        if (*p == ':') p++;
        skipWs(p);
        o.entries.push_back({key, parseVal(p)});
        skipWs(p);
        if (*p == ',') p++;
        skipWs(p);
    }
    if (*p == '}') p++;
    return o;
}

static JArr parseArr(const char*& p) {
    JArr a;
    skipWs(p);
    if (*p != '[') return a;
    p++;
    skipWs(p);
    while (*p && *p != ']') {
        a.items.push_back(parseVal(p));
        skipWs(p);
        if (*p == ',') p++;
        skipWs(p);
    }
    if (*p == ']') p++;
    return a;
}

static JVal parseVal(const char*& p) {
    JVal v;
    skipWs(p);
    if (*p == '"') { v.type = JVal::STR; v.str = parseStr(p); }
    else if (*p == '{') { v.type = JVal::OBJ; v.obj = parseObj(p); }
    else if (*p == '[') { v.type = JVal::ARR; v.arr = parseArr(p); }
    else if (*p == 't') { v.type = JVal::STR; v.str = "true"; p += 4; }
    else if (*p == 'f') { v.type = JVal::STR; v.str = "false"; p += 5; }
    else if (*p == 'n') { p += 4; }
    else { v.type = JVal::NUM; char* end; v.num = strtod(p, &end); if (end == p) v.num = 0; else p = end; }
    return v;
}

static const JVal* objGet(const JObj& o, const char* key) {
    for (auto& e : o.entries) if (e.first == key) return &e.second;
    return nullptr;
}

static double numOr(const JVal* v, double def) { return (v && v->type == JVal::NUM) ? v->num : def; }
static const JObj* objOr(const JVal* v) { return (v && v->type == JVal::OBJ) ? &v->obj : nullptr; }

bool FontAtlas::load(const std::string& jsonPath, const std::string& pngPath) {
    std::ifstream f(jsonPath);
    if (!f.is_open()) {
        std::cerr << "[FontAtlas] JSON not found: " << jsonPath << "\n";
        return false;
    }
    std::string jsonStr((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    const char* p = jsonStr.c_str();
    JVal root = parseVal(p);

    const JObj* atlas = objOr(objGet(root.obj, "atlas"));
    if (atlas) {
        m_atlasWidth    = (float)numOr(objGet(*atlas, "width"), 512);
        m_atlasHeight   = (float)numOr(objGet(*atlas, "height"), 512);
        m_fontSize      = (float)numOr(objGet(*atlas, "size"), 32);
        m_distanceRange = (float)numOr(objGet(*atlas, "distanceRange"), 4);
        auto* yo = objGet(*atlas, "yOrigin");
        if (yo && yo->type == JVal::STR) m_yOriginBottom = (yo->str == "bottom");
    }

    const JObj* metrics = objOr(objGet(root.obj, "metrics"));
    if (metrics) {
        float lh = (float)numOr(objGet(*metrics, "lineHeight"), 1.2);
        m_lineHeight = lh * m_fontSize;
        m_ascender   = (float)numOr(objGet(*metrics, "ascender"), 0.95f);
    }

    auto* glyphsVal = objGet(root.obj, "glyphs");
    if (glyphsVal && glyphsVal->type == JVal::ARR) {
        for (auto& gv : glyphsVal->arr.items) {
            if (gv.type != JVal::OBJ) continue;
            const JObj& g = gv.obj;

            int unicode = -1;
            auto* uv = objGet(g, "unicode"); if (uv && uv->type == JVal::NUM) unicode = (int)uv->num;
            if (unicode < 0) { uv = objGet(g, "char"); if (uv && uv->type == JVal::NUM) unicode = (int)uv->num; }
            if (unicode < 0) { uv = objGet(g, "id"); if (uv && uv->type == JVal::NUM) unicode = (int)uv->num; }
            if (unicode < 0) continue;

            float advance = (float)numOr(objGet(g, "advance"), 0) * m_fontSize;
            if (advance == 0) advance = (float)numOr(objGet(g, "xadvance"), 0);

            float x=0, y=0, w=0, h=0, xOff=0, yOff=0;

            const JObj* ab = objOr(objGet(g, "atlasBounds"));
            if (ab) {
                float left   = (float)numOr(objGet(*ab, "left"), 0);
                float bottom = (float)numOr(objGet(*ab, "bottom"), 0);
                float right  = (float)numOr(objGet(*ab, "right"), 0);
                float top    = (float)numOr(objGet(*ab, "top"), 0);
                x = left; w = right - left; h = top - bottom;
                y = bottom;
            } else {
                x = (float)numOr(objGet(g, "x"), 0);
                y = (float)numOr(objGet(g, "y"), 0);
                w = (float)numOr(objGet(g, "width"), 0);
                h = (float)numOr(objGet(g, "height"), 0);
            }

            const JObj* pb = objOr(objGet(g, "planeBounds"));
            if (pb) {
                xOff = (float)numOr(objGet(*pb, "left"), 0) * m_fontSize;
                yOff = (m_ascender - (float)numOr(objGet(*pb, "top"), 0)) * m_fontSize;
            } else {
                xOff = (float)numOr(objGet(g, "xoffset"), 0);
                yOff = (float)numOr(objGet(g, "yoffset"), 0);
            }

            m_glyphs.emplace(unicode, Glyph(unicode, x, y, w, h, xOff, yOff, advance, m_atlasWidth, m_atlasHeight));
        }
    }

    int ch;
    stbi_set_flip_vertically_on_load(1);
    m_pngData = stbi_load(pngPath.c_str(), &m_pngWidth, &m_pngHeight, &ch, 3);
    stbi_set_flip_vertically_on_load(0);
    if (!m_pngData) {
        std::cerr << "[FontAtlas] PNG not found: " << pngPath << "\n";
        return false;
    }

    m_loaded = true;
    std::cout << "[FontAtlas] Loaded: " << jsonPath << " (" << m_glyphs.size() << " glyphs, "
              << m_pngWidth << "x" << m_pngHeight << ")\n";
    auto* testG = getGlyph(66);
    if (testG) {
        std::cout << "[FontAtlas] Glyph 'B': uv=(" << testG->u0 << "," << testG->v0 << ")-("
                  << testG->u1 << "," << testG->v1 << ") wh=(" << testG->width << "," << testG->height
                  << ") adv=" << testG->xAdvance << " off=(" << testG->xOffset << "," << testG->yOffset << ")\n";
    } else {
        std::cout << "[FontAtlas] Glyph 'B' NOT FOUND\n";
    }
    return true;
}

void FontAtlas::uploadTexture() {
    if (m_textureUploaded || !m_pngData) return;

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_pngWidth, m_pngHeight, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, m_pngData);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(m_pngData);
    m_pngData = nullptr;
    m_textureUploaded = true;
}

const Glyph* FontAtlas::getGlyph(int codePoint) const {
    auto it = m_glyphs.find(codePoint);
    return it != m_glyphs.end() ? &it->second : nullptr;
}

bool FontAtlas::hasGlyph(int codePoint) const {
    return m_glyphs.count(codePoint) > 0;
}

} // namespace
