#include "screens/GameScreen.h"
#include "core/Core.h"
#include "world/WorldManager.h"
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
    std::cout << "[GameScreen] Enter with seed=" << m_seed << " world=" << m_worldName << "\n";
    m_time = 0;
    if (m_core && m_seed != 0) {
        m_core->setSeed(m_seed);
    }
    if (m_core && !m_worldName.empty()) {
        world::PlayerPos pos = world::WorldManager::loadPlayerPos(m_worldName);
        if (pos.x != 0.0f || pos.y != 0.0f || pos.z != 0.0f) {
            auto& mv = m_core->getMovement();
            mv.getState().position = {pos.x, pos.y, pos.z};
            mv.setYaw(pos.yaw);
            mv.setPitch(pos.pitch);
        }
    }
}

void GameScreen::onExit() {
    std::cout << "[GameScreen] Exit\n";
    if (m_core && !m_worldName.empty()) {
        auto& mv = m_core->getMovement();
        auto& st = mv.getState();
        world::PlayerPos pos;
        pos.x = st.position.x;
        pos.y = st.position.y;
        pos.z = st.position.z;
        pos.yaw = mv.getYaw();
        pos.pitch = mv.getPitch();
        world::WorldManager::savePlayerPos(m_worldName, pos);
    }
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
#else
    auto& ts = screens::getTouchState();
    if (ts.backPressed && !m_escPressed) {
        m_nextScreen = ScreenID::Menu;
    }
    m_escPressed = ts.backPressed;
#endif
}

void GameScreen::render(float /*time*/) {
}

} // namespace be::void_::screens
