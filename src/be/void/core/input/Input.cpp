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
 * Реализация — маппинг GLFW → Key, уведомление слушателя.
 */

#include "core/input/Input.h"
#include <algorithm>

namespace be::void_::core::input {

Input::Input() = default;
Input::~Input() = default;

static Key toKey(int glfwKey) {
    switch (glfwKey) {
        case 87:  case 265: return Key::W;
        case 83:  case 264: return Key::S;
        case 65:  case 263: return Key::A;
        case 68:  case 262: return Key::D;
        case 32:             return Key::Space;
        case 256:            return Key::Escape;
        case 340:            return Key::LeftShift;
        default:             return Key::Unknown;
    }
}

void Input::onKey(int glfwKey, int glfwAction) {
    Key key = toKey(glfwKey);
    if (key == Key::Unknown) return;

    KeyAction action = static_cast<KeyAction>(glfwAction);
    if (glfwKey >= 0 && glfwKey < 512) {
        m_keys[static_cast<int>(glfwKey)] = (action != KeyAction::Release);
    }
    if (m_listener) {
        m_listener->onKey(key, action);
    }
}

void Input::onMouseMove(double dx, double dy) {
    if (m_listener) {
        m_listener->onMouseMove(static_cast<float>(dx), static_cast<float>(dy));
    }
}

bool Input::isKeyDown(Key key) const {
    int k = static_cast<int>(key);
    if (k >= 0 && k < 512) return m_keys[k];
    return false;
}

} // namespace be::void_::core::input
