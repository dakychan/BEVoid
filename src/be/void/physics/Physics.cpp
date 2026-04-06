/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#include "physics/Physics.h"
#include <cmath>
#include <algorithm>

namespace be::void_::physics {

void Physics::step(PhysicsState& state, Vec3 inputAccel, float dt, bool wantJump, float terrainHeight) {
    if (dt <= 0.0f || dt > 0.1f) return;

    Vec3 accel = inputAccel;

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
        state.velocity.x *= AIR_DRAG;
        state.velocity.z *= AIR_DRAG;
    }

    accel.y = GRAVITY;

    if (wantJump && state.onGround) {
        state.velocity.y = JUMP_VEL;
        state.onGround = false;
    }

    state.velocity.x += accel.x * dt;
    state.velocity.y += accel.y * dt;
    state.velocity.z += accel.z * dt;

    float hSpeed = std::sqrt(state.velocity.x * state.velocity.x +
                             state.velocity.z * state.velocity.z);
    if (hSpeed > MAX_SPEED) {
        float scale = MAX_SPEED / hSpeed;
        state.velocity.x *= scale;
        state.velocity.z *= scale;
    }

    state.position.x += state.velocity.x * dt;
    state.position.y += state.velocity.y * dt;
    state.position.z += state.velocity.z * dt;

    float groundY = terrainHeight + 1.7f;
    
    // Плавное приземление вместо телепорта
    if (state.position.y <= groundY) {
        // Если игрок близко к земле, плавно опускаем
        if (state.position.y > groundY - 0.5f) {
            state.position.y = groundY;
        } else {
            // Если падает быстро, просто останавливаем
            state.position.y = groundY;
        }
        state.velocity.y = 0;
        state.onGround = true;
    } else {
        state.onGround = false;
    }
}

} // namespace be::void_::physics
