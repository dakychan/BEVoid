/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#include "core/movement/Movement.h"
#include <cmath>
#include <algorithm>

namespace be::void_::core::movement {

Movement::Movement() = default;
Movement::~Movement() = default;

void Movement::onKey(input::Key key, input::KeyAction action) {
    bool pressed = (action == input::KeyAction::Press);
    switch (key) {
        case input::Key::W: m_w = pressed; break;
        case input::Key::S: m_s = pressed; break;
        case input::Key::A: m_a = pressed; break;
        case input::Key::D: m_d = pressed; break;
        case input::Key::Space: m_space = pressed; break;
        case input::Key::LeftShift: m_shift = pressed; break;
        default: break;
    }
}

void Movement::onMouseMove(float dx, float dy) {
    m_yaw   -= dx * MOUSE_SENS;
    m_pitch -= dy * MOUSE_SENS;
    m_pitch = std::clamp(m_pitch, -1.5f, 1.5f);
}

Vec3 Movement::getCameraDir() const {
    float cp = std::cos(m_pitch);
    return { cp * std::sin(m_yaw), std::sin(m_pitch), cp * std::cos(m_yaw) };
}

void Movement::update(float dt, physics::Physics& phys, float terrainHeight) {
    if (dt <= 0.0f || dt > 0.1f) return;

    // Приседание — плавный offset камеры
    float targetCrouch = m_shift ? CROUCH_HEIGHT : 0.0f;
    m_crouchOffset += (targetCrouch - m_crouchOffset) * std::min(1.0f, CROUCH_SPEED * dt);

    // Shift — замедление (красться)
    float speedMul = m_shift ? SLOW_WALK_FACTOR : 1.0f;

    float cy = std::cos(m_yaw);
    float sy = std::sin(m_yaw);
    Vec3 forward = { -sy, 0, -cy };
    Vec3 right   = { -cy, 0, sy };

    Vec3 inputAccel = {0, 0, 0};
    if (m_w) { inputAccel = inputAccel + forward * MOVE_ACCEL * speedMul; }
    if (m_s) { inputAccel = inputAccel - forward * MOVE_ACCEL * speedMul; }
    if (m_a) { inputAccel = inputAccel + right * MOVE_ACCEL * speedMul; }
    if (m_d) { inputAccel = inputAccel - right * MOVE_ACCEL * speedMul; }

    phys.step(m_state, inputAccel, dt, m_space, terrainHeight);
}

} // namespace be::void_::core::movement
