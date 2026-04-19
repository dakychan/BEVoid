/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#ifndef BEVOID_MOVEMENT_H
#define BEVOID_MOVEMENT_H

#include "Vec3.h"
#include "core/input/Input.h"
#include "physics/Physics.h"
#include <cstdint>

namespace be::void_::core::movement {

class Movement : public input::IInputListener {
public:
    Movement();
    ~Movement() override;

    void update(float deltaTime, physics::Physics& phys, float terrainHeight = 0.0f);

    void onKey(input::Key key, input::KeyAction action) override;
    void onMouseMove(float dx, float dy) override;

    Vec3 getCameraPos() const { return m_state.position; }
    Vec3 getCameraDir() const;
    float getYaw()   const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    float getCrouchOffset() const { return m_crouchOffset; }
    bool  onGround() const { return m_state.onGround; }

    void setYaw(float y) { m_yaw = y; }
    void setPitch(float p) { m_pitch = p; }

    physics::PhysicsState& getState() { return m_state; }

private:
    physics::PhysicsState m_state;
    float m_yaw   = 0.0f;
    float m_pitch = -0.2f;

    bool m_w = false, m_s = false;
    bool m_a = false, m_d = false;
    bool m_space = false;
    bool m_shift = false;

    float m_crouchOffset = 0.0f;

    static constexpr float MOVE_ACCEL = 40.0f;
    static constexpr float MOUSE_SENS = 0.0005f;
    static constexpr float CROUCH_HEIGHT = 1.2f;
    static constexpr float CROUCH_SPEED = 5.0f;
    static constexpr float SLOW_WALK_FACTOR = 0.5f;  // Shift = скорость / 2
};

} // namespace be::void_::core::movement

#endif // BEVOID_MOVEMENT_H
