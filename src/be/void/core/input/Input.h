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
 * Модуль ввода — отдельные хендлеры клавиатуры и мыши.
 * Movement подписывается на Input через callback-интерфейс.
 */

#ifndef BEVOID_INPUT_H
#define BEVOID_INPUT_H

#include <cstdint>
#include <functional>

namespace be::void_::core::input {

/* Коды клавиш ( GLFW-совместимые ) */
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

/* Подписчик на ввод */
struct IInputListener {
    virtual void onKey(Key key, KeyAction action) = 0;
    virtual void onMouseMove(float dx, float dy)  = 0;
    virtual ~IInputListener() = default;
};

class Input {
public:
    Input();
    ~Input();

    /* Подписка — один слушатель (Movement) */
    void setListener(IInputListener* listener) { m_listener = listener; }

    /* Вызываются из Game (GLFW callbacks) */
    void onKey(int glfwKey, int glfwAction);
    void onMouseMove(double dx, double dy);

    /* Текущее состояние клавиш (для Movement::update) */
    bool isKeyDown(Key key) const;

private:
    IInputListener* m_listener = nullptr;
    bool m_keys[512] = {}; /* состояние всех клавиш */
};

} // namespace be::void_::core::input

#endif // BEVOID_INPUT_H
