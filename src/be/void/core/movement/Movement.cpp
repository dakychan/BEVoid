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
 * Заглушка — в будущем WASD, мышь, камера.
 */

#include "core/movement/Movement.h"

namespace be::void_::core::movement {

Movement::Movement() = default;
Movement::~Movement() = default;

void Movement::update(float /*deltaTime*/) {
    /* TODO: обработка ввода, WASD, мышь */
}

} // namespace be::void_::core::movement
