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
 * be.void.core.movement
 *
 * Подписывается на Input, рассчитывает ускорение от WASD,
 * передаёт в Physics для расчёта гравитации/прыжка/трения.
 */

#ifndef BEVOID_MOVEMENT_H
#define BEVOID_MOVEMENT_H

#include "core/input/Input.h"
#include "physics/Physics.h"
#include <cstdint>

namespace be::void_::core::movement {

struct Vec3 {
    float x = 0, y = 0, z = 0;
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)      const { return {x*s, y*s, z*s}; }
};

class Movement : public input::IInputListener {
public:
    Movement();
    ~Movement() override;

    /* Обновить — рассчитывает ускорение от ввода, Physics считает остальное */
    void update(float deltaTime, physics::Physics& phys, float terrainHeight = 0.0f);

    /* IInputListener */
    void onKey(input::Key key, input::KeyAction action) override;
    void onMouseMove(float dx, float dy) override;

    /* Позиция и направление камеры */
    Vec3 getCameraPos() const { return {m_state.position.x, m_state.position.y, m_state.position.z}; }
    Vec3 getCameraDir() const;
    float getYaw()   const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    bool  onGround() const { return m_state.onGround; }

    /* Доступ к физическому состоянию (для отладки) */
    physics::PhysicsState& getState() { return m_state; }

private:
    /* Физическое состояние (Physics обновляет) */
    physics::PhysicsState m_state;

    /* Камера */
    float m_yaw   = 0.0f;
    float m_pitch = -0.2f;

    /* Управление */
    bool m_w = false, m_s = false;
    bool m_a = false, m_d = false;
    bool m_space = false;

    static constexpr float MOVE_ACCEL = 20.0f;  /* м/с² */
    static constexpr float MOUSE_SENS = 0.0005f;
};

} // namespace be::void_::core::movement

#endif // BEVOID_MOVEMENT_H
