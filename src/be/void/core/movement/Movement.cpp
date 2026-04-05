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
 * Физика: WASD → ускорение, трение, гравитация, прыжок.
 * Камера от первого лица.
 */

#include "core/movement/Movement.h"
#include <cmath>
#include <algorithm>
#include <cstdint>

namespace be::void_::core::movement {

Movement::Movement() = default;
Movement::~Movement() = default;

void Movement::onKey(int key, bool pressed) {
    switch (key) {
        case 87:  case 265: m_w     = pressed; break; /* W / Up */
        case 83:  case 264: m_s     = pressed; break; /* S / Down */
        case 65:  case 263: m_a     = pressed; break; /* A / Left */
        case 68:  case 262: m_d     = pressed; break; /* D / Right */
        case 32:              m_space = pressed; break; /* Space */
    }
}

void Movement::onMouseMove(float dx, float dy) {
    m_yaw   += dx * MOUSE_SENS;
    m_pitch += dy * MOUSE_SENS;

    /* Clamp pitch — не даём камере перевернуться */
    m_pitch = std::clamp(m_pitch, -1.5f, 1.5f); /* ~±86° */
}

Vec3 Movement::getCameraDir() const {
    /* Forward vector из yaw/pitch */
    float cp = std::cos(m_pitch);
    return {
        cp * std::sin(m_yaw),
        std::sin(m_pitch),
        cp * std::cos(m_yaw)
    };
}

void Movement::update(float dt) {
    if (dt <= 0.0f || dt > 0.1f) return;

    /* --- Направление камеры (горизонтальное) --- */
    float cy = std::cos(m_yaw);
    float sy = std::sin(m_yaw);

    /* Векторы forward и right (только XZ) */
    Vec3 forward = { sy, 0, cy };
    Vec3 right   = { cy, 0, -sy };

    /* --- Ускорение от клавиш --- */
    Vec3 accel = {0, 0, 0};

    if (m_w)     accel = accel + forward * MOVE_ACCEL;
    if (m_s)     accel = accel - forward * MOVE_ACCEL;
    if (m_a)     accel = accel - right * MOVE_ACCEL;
    if (m_d)     accel = accel + right * MOVE_ACCEL;

    /* --- Трение (торможение на земле) --- */
    if (m_onGround) {
        Vec3 velXZ = {m_vel.x, 0, m_vel.z};
        float speed = std::sqrt(velXZ.x*velXZ.x + velXZ.z*velXZ.z);
        if (speed > 0.01f) {
            float friction = FRICTION * speed;
            Vec3 frictionDir = velXZ / speed;
            accel = accel - frictionDir * friction;
        }
    } else {
        /* В воздухе — небольшое сопротивление */
        m_vel.x *= 0.98f;
        m_vel.z *= 0.98f;
    }

    /* --- Гравитация --- */
    accel.y = GRAVITY;

    /* --- Прыжок --- */
    if (m_space && m_onGround) {
        m_vel.y = JUMP_VEL;
        m_onGround = false;
    }

    /* --- Интеграция Эйлера --- */
    m_vel = m_vel + accel * dt;

    /* Clamp скорости — не даём ускориться до бесконечности */
    float hSpeed = std::sqrt(m_vel.x*m_vel.x + m_vel.z*m_vel.z);
    if (hSpeed > MOVE_SPEED * 1.5f) {
        float scale = (MOVE_SPEED * 1.5f) / hSpeed;
        m_vel.x *= scale;
        m_vel.z *= scale;
    }

    /* Позиция */
    m_pos = m_pos + m_vel * dt;

    /* --- Пол (земля на Y=0) --- */
    float groundLevel = 0.0f;
    if (m_pos.y <= groundLevel) {
        m_pos.y = groundLevel;
        m_vel.y = 0;
        m_onGround = true;
    }
}

} // namespace be::void_::core::movement
