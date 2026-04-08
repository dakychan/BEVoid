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
 * be.void.physics.cycles
 *
 * Реализация цикла дня/ночи:
 * - Солнце движется по дуге
 * - Закат/рассвет — плавная смена цвета (оранжевый→голубой→чёрный)
 * - Луна с фазами
 * - Небо меняет цвет: чёрный→тёмно-синий→голубой→оранжевый→голубой→тёмно-синий→чёрный
 */

#include "physics/Cycles.h"
#include <algorithm>

namespace be::void_::physics {

Cycles::Cycles() = default;
Cycles::~Cycles() = default;

void Cycles::setTimezone(float utcOffset) {
    m_timezone = utcOffset;
}

void Cycles::update(float deltaTime) {
    m_timeOfDay += deltaTime;
    if (m_timeOfDay >= DAY_LENGTH) m_timeOfDay -= DAY_LENGTH;

    /* Час от 0 до 24 */
    float hours = (m_timeOfDay / DAY_LENGTH) * 24.0f;
    m_state.dayProgress = m_timeOfDay / DAY_LENGTH;

    /* Угол солнца: полночь=0, полдень=180° */
    float hourAngle = (hours / 24.0f) * 360.0f - 90.0f; /* -90 чтобы в 6:00 солнце всходило */

    calcSun(hourAngle);
    calcMoon(hourAngle);
    calcSkyColor();

    m_state.isDaytime = m_state.sunIntensity > 0.3f;
}

void Cycles::calcSun(float angleDeg) {
    float rad = angleDeg * 3.14159f / 180.0f;

    m_state.sunX = std::cos(rad);
    m_state.sunY = std::sin(rad);
    m_state.sunZ = 0.3f; /* слегка сбоку */

    /* Нормализация */
    float len = std::sqrt(m_state.sunX*m_state.sunX + m_state.sunY*m_state.sunY + m_state.sunZ*m_state.sunZ);
    m_state.sunX /= len; m_state.sunY /= len; m_state.sunZ /= len;

    /* Интенсивность: sunY > 0 = день, < 0 = ночь */
    m_state.sunIntensity = std::max(0.0f, m_state.sunY);

    /* Цвет солнца: плавный градиент по высоте */
    if (m_state.sunY > 0.3f) {
        /* Полдень — чистый белый */
        m_state.sunColorR = 1.0f;
        m_state.sunColorG = 0.95f;
        m_state.sunColorB = 0.85f;
    } else if (m_state.sunY > 0.05f) {
        /* День — тёплый жёлтый */
        float t = (m_state.sunY - 0.05f) / 0.25f; // 0..1
        m_state.sunColorR = 1.0f;
        m_state.sunColorG = 0.80f + 0.15f * t;
        m_state.sunColorB = 0.55f + 0.30f * t;
    } else if (m_state.sunY > 0.0f) {
        /* Закат/рассвет — яркий розовый/оранжевый */
        float t = m_state.sunY / 0.05f; // 0..1
        m_state.sunColorR = 1.0f;
        m_state.sunColorG = 0.35f + 0.45f * t;
        m_state.sunColorB = 0.20f + 0.35f * t;
    } else {
        /* Ночь — солнце не видно */
        m_state.sunColorR = 0.0f;
        m_state.sunColorG = 0.0f;
        m_state.sunColorB = 0.0f;
    }

    /* Цвет луны — мягкий голубовато-белый */
    m_state.moonColorR = 0.80f;
    m_state.moonColorG = 0.82f;
    m_state.moonColorB = 0.95f;

    /* Ambient — рассеянный свет (повышен) */
    m_state.ambientIntensity = 0.35f + m_state.sunIntensity * 0.65f;
}

void Cycles::calcMoon(float /*sunAngle*/) {
    /* Луна противоположна солнцу + фаза */
    float moonAngle = m_state.dayProgress * 3.14159f * 2.0f + 3.14159f; /* противоположно */

    m_state.moonX = std::cos(moonAngle);
    m_state.moonY = std::sin(moonAngle) * 0.7f; /* ниже солнца */
    m_state.moonZ = -0.3f;

    float len = std::sqrt(m_state.moonX*m_state.moonX + m_state.moonY*m_state.moonY + m_state.moonZ*m_state.moonZ);
    if (len > 0) { m_state.moonX /= len; m_state.moonY /= len; m_state.moonZ /= len; }

    /* Фаза луны — медленный цикл 29.5 дней */
    float moonPhaseAngle = (m_timeOfDay / m_moonCycle) * 3.14159f * 2.0f;
    m_state.moonPhase = (std::cos(moonPhaseAngle) * 0.5f + 0.5f); /* 0..1 */
}

void Cycles::calcSkyColor() {
    float y = m_state.sunY; /* -1..1 */

    if (y > 0.2f) {
        /* День — голубое небо */
        m_state.skyR = 0.25f; m_state.skyG = 0.45f; m_state.skyB = 0.90f;
        m_state.horizonR = 0.55f; m_state.horizonG = 0.70f; m_state.horizonB = 0.95f;
    } else if (y > 0.0f) {
        /* Рассвет/закат — плавный переход */
        float t = y / 0.2f;
        m_state.skyR = 0.85f * (1.0f - t) + 0.25f * t;
        m_state.skyG = 0.40f * (1.0f - t) + 0.45f * t;
        m_state.skyB = 0.25f * (1.0f - t) + 0.90f * t;
        m_state.horizonR = 0.90f * (1.0f - t) + 0.55f * t;
        m_state.horizonG = 0.50f * (1.0f - t) + 0.70f * t;
        m_state.horizonB = 0.30f * (1.0f - t) + 0.95f * t;
    } else if (y > -0.15f) {
        /* Сумерки — тёмно-синий */
        float t = (y + 0.15f) / 0.15f;
        m_state.skyR = 0.05f * (1.0f - t) + 0.25f * t;
        m_state.skyG = 0.05f * (1.0f - t) + 0.45f * t;
        m_state.skyB = 0.15f * (1.0f - t) + 0.90f * t;
        m_state.horizonR = 0.10f * (1.0f - t) + 0.55f * t;
        m_state.horizonG = 0.10f * (1.0f - t) + 0.70f * t;
        m_state.horizonB = 0.20f * (1.0f - t) + 0.95f * t;
    } else {
        /* Ночь */
        m_state.skyR = 0.02f; m_state.skyG = 0.02f; m_state.skyB = 0.06f;
        m_state.horizonR = 0.04f; m_state.horizonG = 0.04f; m_state.horizonB = 0.08f;
    }

    /* Туман тот же что и небо */
    m_state.fogR = m_state.skyR;
    m_state.fogG = m_state.skyG;
    m_state.fogB = m_state.skyB;
}

} // namespace be::void_::physics
