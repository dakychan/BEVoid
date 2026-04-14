/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

/*
 * be.void.core.input
 *
 * Модуль ввода — поддержка нескольких слушателей.
 * Movement и другие подсистемы подписываются через addListener().
 */

#ifndef BEVOID_INPUT_H
#define BEVOID_INPUT_H

#include <cstdint>
#include <functional>
#include <vector>

namespace be::void_::core::input {

enum class Key : int32_t {
    W = 87, S = 83, A = 65, D = 68,
    Space = 32, Escape = 256,
    Up = 265, Down = 264, Left = 263, Right = 262,
    LeftShift = 340,
    Unknown = -1
};

enum class KeyAction : int32_t {
    Release = 0,
    Press   = 1,
    Repeat  = 2,
};

struct IInputListener {
    virtual void onKey(Key key, KeyAction action) = 0;
    virtual void onMouseMove(float dx, float dy)  = 0;
    virtual ~IInputListener() = default;
};

class Input {
public:
    Input();
    ~Input();

    void addListener(IInputListener* listener);
    void removeListener(IInputListener* listener);
    void setListener(IInputListener* listener);

    void onKey(int glfwKey, int glfwAction);
    void onMouseMove(double dx, double dy);

    bool isKeyDown(Key key) const;

private:
    std::vector<IInputListener*> m_listeners;
    bool m_keys[512] = {};
};

} // namespace be::void_::core::input

#endif // BEVOID_INPUT_H
