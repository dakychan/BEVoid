#include "screens/Screen.h"
#include "screens/MenuScreen.h"
#include "screens/GameScreen.h"
#include "core/Core.h"

namespace be::void_::screens {

std::unique_ptr<Screen> ScreenManager::createScreen(ScreenID id) {
    switch (id) {
        case ScreenID::Menu: return std::make_unique<MenuScreen>();
        case ScreenID::Game: {
            auto gs = std::make_unique<GameScreen>();
            gs->setCore(m_core);
            return gs;
        }
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
        setScreen(createScreen(next));
    }
}

void ScreenManager::render(float time) {
    if (m_screen) m_screen->render(time);
}

} // namespace be::void_::screens
