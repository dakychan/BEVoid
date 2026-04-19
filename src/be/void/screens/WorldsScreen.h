#ifndef BEVOID_WORLDS_SCREEN_H
#define BEVOID_WORLDS_SCREEN_H

#include "screens/Screen.h"
#include "core/render/font/FontRenderer.h"
#include "world/WorldMeta.h"
#include <vector>
#include <string>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#endif

namespace be::void_::screens {

class WorldsScreen : public Screen {
public:
    ScreenID id() const override { return ScreenID::Worlds; }

    void onEnter() override;
    void onExit()  override;

    void update(float dt) override;
    void render(float time) override;

    ScreenID nextScreen() const override { return m_nextScreen; }

    void setWorldSeed(uint32_t seed) { m_selectedSeed = seed; }
    uint32_t getWorldSeed() const { return m_selectedSeed; }
    uint32_t getGameSeed() const override { return m_selectedSeed; }
    const std::string& getWorldName() const { return m_selectedName; }

private:
    void refreshWorlds();
    void drawWorldEntry(int idx, float y, bool selected);
    void drawBottomButton(float x1, float y1, float x2, float y2, const std::string& text, float r, float g, float b);
    void loadPreviewTexture(int idx);

    std::vector<world::WorldMeta> m_worlds;
    int m_selected = 0;
    ScreenID m_nextScreen = ScreenID::None;
    uint32_t m_selectedSeed = 0;
    std::string m_selectedName;

    core::render::font::FontRenderer m_fontRenderer;
    bool m_fontsLoaded = false;

    bool m_upPressed = false;
    bool m_downPressed = false;
    bool m_escPressed = false;

    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    bool m_mouseLeftPressed = false;
    int m_hoveredItem = -1;

    struct PreviewTex { GLuint tex; int w, h; };
    std::vector<PreviewTex> m_previews;
};

} // namespace

#endif
