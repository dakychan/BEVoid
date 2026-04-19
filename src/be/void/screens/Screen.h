#ifndef BEVOID_SCREEN_H
#define BEVOID_SCREEN_H

#include <memory>
#include <cstdint>
#include <string>

namespace be::void_ {
namespace core { class Core; }
namespace screens {

struct TouchState {
    float ndcX = 0.0f;
    float ndcY = 0.0f;
    bool touched = false;
    bool tapped = false;
    bool backPressed = false;
};

TouchState& getTouchState();

enum class ScreenID {
    None,
    Menu,
    Game,
    Worlds,
    NewWorld
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
    void setPendingSeed(uint32_t seed) { m_pendingSeed = seed; }
    uint32_t pendingSeed() const { return m_pendingSeed; }
    void setPendingWorldName(const std::string& name) { m_pendingWorldName = name; }
    const std::string& pendingWorldName() const { return m_pendingWorldName; }

private:
    std::unique_ptr<Screen> m_screen;
    core::Core* m_core = nullptr;
    uint32_t m_pendingSeed = 0;
    std::string m_pendingWorldName;
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
    virtual uint32_t getGameSeed() const { return 0; }
    virtual const std::string& getWorldName() const { static std::string empty; return empty; }
};

} // namespace screens
} // namespace be::void_

#endif // BEVOID_SCREEN_H
