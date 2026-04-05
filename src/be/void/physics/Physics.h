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
 * be.void.physics
 *
 * Рассчитывает физику: гравитация, прыжок, трение, коллизии с землёй.
 * Movement вызывает Physics::step() каждый кадр.
 */

#ifndef BEVOID_PHYSICS_H
#define BEVOID_PHYSICS_H

#include <cstdint>

namespace be::void_::physics {

struct Vec3 {
    float x = 0, y = 0, z = 0;
};

struct PhysicsState {
    Vec3 position  = {0, 50.0f, -4.0f}; /* высоко над землёй */
    Vec3 velocity  = {0, 0, 0};
    bool onGround  = true;
};

class Physics {
public:
    Physics() = default;
    ~Physics() = default;

    /* Рассчитать физику на dt.
       inputAccel — ускорение от ввода (WASD).
       terrainHeight — высота террейна в точке игрока. */
    void step(PhysicsState& state, Vec3 inputAccel, float dt, bool wantJump, float terrainHeight);

    /* Константы */
    static constexpr float GRAVITY    = -20.0f;
    static constexpr float FRICTION   = 10.0f;
    static constexpr float AIR_DRAG   = 0.98f;
    static constexpr float JUMP_VEL   = 7.0f;
    static constexpr float MAX_SPEED  = 6.0f;  /* м/с */
    static constexpr float GROUND_Y  = 0.0f;
};

} // namespace be::void_::physics

#endif // BEVOID_PHYSICS_H
