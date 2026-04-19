#ifndef BEVOID_FONT_GLYPH_H
#define BEVOID_FONT_GLYPH_H

#include <cstdint>

namespace be::void_::core::render::font {

struct Glyph {
    int32_t id;
    float x, y;
    float width, height;
    float xOffset, yOffset;
    float xAdvance;
    float u0, v0, u1, v1;

    Glyph()
        : id(0), x(0), y(0), width(0), height(0)
        , xOffset(0), yOffset(0), xAdvance(0)
        , u0(0), v0(0), u1(0), v1(0) {}

    Glyph(int32_t id, float x, float y, float w, float h,
          float xOff, float yOff, float adv,
          float atlasW, float atlasH)
        : id(id), x(x), y(y), width(w), height(h)
        , xOffset(xOff), yOffset(yOff), xAdvance(adv)
        , u0(x / atlasW), v0(y / atlasH)
        , u1((x + w) / atlasW), v1((y + h) / atlasH) {}
};

} // namespace

#endif
