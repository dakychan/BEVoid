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
 * Рассчитывает: гравитация → трение → интеграция Эйлера → коллизия с землёй.
 */

#include "physics/Physics.h"
#include <cmath>
#include <algorithm>

namespace be::void_::physics {

void Physics::step(PhysicsState& state, Vec3 inputAccel, float dt, bool wantJump, float terrainHeight) {
    if (dt <= 0.0f || dt > 0.1f) return;

    Vec3 accel = inputAccel;

    /* Трение (на земле — сильное, в воздухе — drag) */
    if (state.onGround) {
        float speed = std::sqrt(state.velocity.x * state.velocity.x +
                                state.velocity.z * state.velocity.z);
        if (speed > 0.01f) {
            float nx = state.velocity.x / speed;
            float nz = state.velocity.z / speed;
            accel.x -= nx * FRICTION * speed;
            accel.z -= nz * FRICTION * speed;
        }
    } else {
        /* Сопротивление воздуха */
        state.velocity.x *= AIR_DRAG;
        state.velocity.z *= AIR_DRAG;
    }

    /* Гравитация */
    accel.y = GRAVITY;

    /* Прыжок */
    if (wantJump && state.onGround) {
        state.velocity.y = JUMP_VEL;
        state.onGround = false;
    }

    /* Интеграция Эйлера */
    state.velocity.x += accel.x * dt;
    state.velocity.y += accel.y * dt;
    state.velocity.z += accel.z * dt;

    /* Clamp горизонтальной скорости */
    float hSpeed = std::sqrt(state.velocity.x * state.velocity.x +
                             state.velocity.z * state.velocity.z);
    if (hSpeed > MAX_SPEED) {
        float scale = MAX_SPEED / hSpeed;
        state.velocity.x *= scale;
        state.velocity.z *= scale;
    }

    /* Позиция */
    state.position.x += state.velocity.x * dt;
    state.position.y += state.velocity.y * dt;
    state.position.z += state.velocity.z * dt;

    /* Коллизия с террейном */
    float groundY = terrainHeight + 1.7f; /* +1.7m высота глаз */
    if (state.position.y <= groundY) {
        state.position.y = groundY;
        state.velocity.y = 0;
        state.onGround = true;
    }
}

} // namespace be::void_::physics
