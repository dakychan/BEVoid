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
 * Физика. Заглушка.
 */

#ifndef BEVOID_PHYSICS_H
#define BEVOID_PHYSICS_H

namespace be::void_::physics {

class Physics {
public:
    Physics() = default;
    ~Physics() = default;

    void update(float deltaTime);
    void step(float dt);

private:
    float m_gravity = -9.81f;
};

} // namespace be::void_::physics

#endif // BEVOID_PHYSICS_H
