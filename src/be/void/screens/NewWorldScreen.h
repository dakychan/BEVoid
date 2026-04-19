#ifndef BEVOID_NEWWORLD_SCREEN_H
#define BEVOID_NEWWORLD_SCREEN_H

#include "screens/Screen.h"
#include "core/render/font/FontRenderer.h"
#include <string>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#endif

namespace be::void_::screens {

class NewWorldScreen : public Screen {
public:
    ScreenID id() const override { return ScreenID::NewWorld; }

    void onEnter() override;
    void onExit()  override;

    void update(float dt) override;
    void render(float time) override;

    ScreenID nextScreen() const override { return m_nextScreen; }

    uint32_t getGameSeed() const override { return m_seed; }

private:
    void regenerateSeed();
    void createAndPlay();
    void drawButton(float x1, float y1, float x2, float y2, const std::string& text, float r, float g, float b);

    uint32_t m_seed = 0;
    std::string m_seedStr;
    std::string m_name;
    ScreenID m_nextScreen = ScreenID::None;
    int m_editing = 0;

    core::render::font::FontRenderer m_fontRenderer;
    bool m_fontsLoaded = false;

    bool m_escPressed = false;
    bool m_enterPressed = false;
    bool m_rPressed = false;

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    bool m_mouseLeftPressed = false;

    static constexpr float FIELD_X1 = -0.35f;
    static constexpr float FIELD_X2 = 0.35f;
    static constexpr float NAME_Y = 0.4f;
    static constexpr float SEED_Y = 0.25f;
    static constexpr float FIELD_HALF_H = 0.025f;
    static constexpr float OUTLINE_W = 0.003f;
};

} // namespace

#endif
