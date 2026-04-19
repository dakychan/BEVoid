#include "screens/GameScreen.h"
#include "core/Core.h"
#include <iostream>

#if defined(BEVOID_PLATFORM_ANDROID)
    #include <GLES3/gl3.h>
#else
    #include <glad/glad.h>
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
#endif

namespace be::void_::screens {

void GameScreen::onEnter() {
    std::cout << "[GameScreen] Enter\n";
    m_time = 0;
}

void GameScreen::onExit() {
    std::cout << "[GameScreen] Exit\n";
}

void GameScreen::update(float dt) {
    if (!m_core) return;
    m_time += dt;
    m_core->update(dt);

#if !defined(BEVOID_PLATFORM_ANDROID)
    if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (!m_escPressed) {
            m_nextScreen = ScreenID::Menu;
            m_escPressed = true;
        }
    } else {
        m_escPressed = false;
    }
#endif
}

void GameScreen::render(float /*time*/) {
}

} // namespace be::void_::screens
