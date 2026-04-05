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
 * Камера и движение.
 * Сейчас: заглушка. В будущем: WASD, мышь, полёт, физика.
 */

#ifndef BEVOID_MOVEMENT_H
#define BEVOID_MOVEMENT_H

#include <cstdint>

namespace be::void_::core::movement {

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

class Movement {
public:
    Movement();
    ~Movement();

    /* Обновить позицию на основе ввода */
    void update(float deltaTime);

    /* Получить позицию камеры */
    Vec3 getCameraPos() const { return m_cameraPos; }

private:
    Vec3 m_cameraPos;
    float m_yaw = 0.0f;
    float m_pitch = 0.0f;
};

} // namespace be::void_::core::movement

#endif // BEVOID_MOVEMENT_H
