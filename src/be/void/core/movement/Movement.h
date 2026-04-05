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
 * Камера от первого лица: WASD + мышь + гравитация + прыжок.
 */

#ifndef BEVOID_MOVEMENT_H
#define BEVOID_MOVEMENT_H

#include <cstdint>

namespace be::void_::core::movement {

struct Vec3 {
    float x = 0, y = 0, z = 0;

    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s)      const { return {x*s, y*s, z*s}; }
    Vec3 operator/(float s)      const { return {x/s, y/s, z/s}; }
};

class Movement {
public:
    Movement();
    ~Movement();

    /* Обновить на основе ввода и дельты времени */
    void update(float deltaTime);

    /* Ввод — вызывать из Game (GLFW callback или Android touch) */
    void onKey(int key, bool pressed);
    void onMouseMove(float dx, float dy);

    /* Получить позицию и направление камеры */
    Vec3 getCameraPos() const { return m_pos; }
    Vec3 getCameraDir() const;  /* нормализованный forward vector */
    float getYaw()   const { return m_yaw; }
    float getPitch() const { return m_pitch; }

    /* Проверка на земле (для UI) */
    bool onGround() const { return m_onGround; }

private:
    /* Физика */
    Vec3 m_pos    = {0, 1.7f, 0};   /* позиция камеры (1.7m = высота глаз) */
    Vec3 m_vel    = {0, 0, 0};      /* скорость */

    /* Управление */
    bool m_w = false, m_s = false;
    bool m_a = false, m_d = false;
    bool m_space = false;

    /* Камера */
    float m_yaw   = 0.0f;    /* горизонтальный угол */
    float m_pitch = 0.0f;    /* вертикальный угол */
    bool  m_onGround = true;

    /* Константы */
    static constexpr float MOVE_SPEED     = 8.0f;     /* м/с */
    static constexpr float MOVE_ACCEL     = 40.0f;    /* м/с² */
    static constexpr float FRICTION       = 8.0f;     /* торможение */
    static constexpr float GRAVITY        = -20.0f;   /* м/с² */
    static constexpr float JUMP_VEL       = 8.0f;     /* м/с */
    static constexpr float MOUSE_SENS     = 0.002f;   /* чувствительность мыши */
};

} // namespace be::void_::core::movement

#endif // BEVOID_MOVEMENT_H
