#include "screens/Screen.h"
#include "screens/MenuScreen.h"
#include "screens/GameScreen.h"
#include "screens/WorldsScreen.h"
#include "screens/NewWorldScreen.h"
#include "core/Core.h"

namespace be::void_::screens {

static TouchState s_touchState;

TouchState& getTouchState() { return s_touchState; }

std::unique_ptr<Screen> ScreenManager::createScreen(ScreenID id) {
    switch (id) {
        case ScreenID::Menu: return std::make_unique<MenuScreen>();
        case ScreenID::Game: {
            auto gs = std::make_unique<GameScreen>();
            gs->setCore(m_core);
            gs->setSeed(m_pendingSeed);
            gs->setWorldName(m_pendingWorldName);
            m_pendingSeed = 0;
            m_pendingWorldName.clear();
            return gs;
        }
        case ScreenID::Worlds: return std::make_unique<WorldsScreen>();
        case ScreenID::NewWorld: return std::make_unique<NewWorldScreen>();
        default: return nullptr;
    }
}

void ScreenManager::setScreen(std::unique_ptr<Screen> screen) {
    if (m_screen) m_screen->onExit();
    m_screen = std::move(screen);
    if (m_screen) m_screen->onEnter();
}

Screen* ScreenManager::currentScreen() const {
    return m_screen.get();
}

void ScreenManager::update(float dt) {
    if (!m_screen) return;
    m_screen->update(dt);

    auto next = m_screen->nextScreen();
    if (next != ScreenID::None) {
        m_pendingSeed = m_screen->getGameSeed();
        m_pendingWorldName = m_screen->getWorldName();
        setScreen(createScreen(next));
    }
}

void ScreenManager::render(float time) {
    if (m_screen) m_screen->render(time);
}

} // namespace be::void_::screens
