#ifndef BEVOID_SCREEN_H
#define BEVOID_SCREEN_H

#include <memory>

namespace be::void_ {
namespace core { class Core; }
namespace screens {

enum class ScreenID {
    None,
    Menu,
    Game
};

class Screen;

class ScreenManager {
public:
    void setScreen(std::unique_ptr<Screen> screen);
    Screen* currentScreen() const;

    void update(float dt);
    void render(float time);

    std::unique_ptr<Screen> createScreen(ScreenID id);

    void setCore(core::Core* core) { m_core = core; }

private:
    std::unique_ptr<Screen> m_screen;
    core::Core* m_core = nullptr;
};

class Screen {
public:
    virtual ~Screen() = default;
    virtual ScreenID id() const = 0;

    virtual void onEnter()  {}
    virtual void onExit()   {}

    virtual void update(float dt) = 0;
    virtual void render(float time) = 0;

    virtual ScreenID nextScreen() const { return ScreenID::None; }
};

} // namespace screens
} // namespace be::void_

#endif // BEVOID_SCREEN_H
