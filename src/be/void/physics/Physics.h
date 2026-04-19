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
    Vec3 position  = {5.0f, 1.7f, 5.0f};
    Vec3 velocity  = {0, 0, 0};
    bool onGround  = false;

    float windSpeed = 2.0f;
    float windDirX  = 1.0f;
    float windDirZ  = 0.0f;
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

    float getWindSpeed() const { return m_windSpeed; }
    static float getDefaultWindSpeed() { return 2.0f; }

private:
    float m_windSpeed = 2.0f;
};

} // namespace be::void_::physics

#endif // BEVOID_PHYSICS_H
