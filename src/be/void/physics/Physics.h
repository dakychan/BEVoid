/*
 * ============================================================
 * BEVoid Project
 * Copyright (c) 2025-2026 BEVoid Project
 * All rights reserved.
 * Licensed under the BEVoid Software License Agreement v1.0
 * See LICENSE and COPYRIGHT files in the repository root.
 * ============================================================
 */

#ifndef BEVOID_PHYSICS_H
#define BEVOID_PHYSICS_H

#include "Vec3.h"
#include <cstdint>

namespace be::void_::physics {

struct PhysicsState {
    Vec3 position  = {0, 50.0f, 0};
    Vec3 velocity  = {0, 0, 0};
    bool onGround  = false;
};

class Physics {
public:
    Physics() = default;
    ~Physics() = default;

    void step(PhysicsState& state, Vec3 inputAccel, float dt, bool wantJump, float terrainHeight);

    static constexpr float GRAVITY    = -20.0f;
    static constexpr float FRICTION   = 10.0f;
    static constexpr float AIR_DRAG   = 0.98f;
    static constexpr float JUMP_VEL   = 10.0f;
    static constexpr float MAX_SPEED  = 15.0f;
    static constexpr float GROUND_Y  = 0.0f;
};

} // namespace be::void_::physics

#endif // BEVOID_PHYSICS_H
