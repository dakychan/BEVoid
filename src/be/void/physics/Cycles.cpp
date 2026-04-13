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
 * - Редкие эвенты: белая ночь, полярная ночь, затмение, полярное сияние
 */

#include "physics/Cycles.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace be::void_::physics {

static float smoothstepC(float edge0, float edge1, float x) {
    float t = (x - edge0) / (edge1 - edge0);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return t * t * (3.0f - 2.0f * t);
}

Cycles::Cycles() {
    std::srand(static_cast<uint32_t>(std::time(nullptr)));
    pickNextEventThreshold();

    /* Init event fields in state */
    m_state.activeEvent = EventType_None;
    m_state.eventName = nullptr;
    m_state.eventProgress = 0.0f;
}
Cycles::~Cycles() = default;

void Cycles::setTimezone(float utcOffset) {
    m_timezone = utcOffset;
}

void Cycles::update(float deltaTime) {
    float dtDays = deltaTime / DAY_LENGTH;

    /* Детектим смену дня */
    m_timeOfDay += deltaTime;
    if (m_timeOfDay >= DAY_LENGTH) {
        m_timeOfDay -= DAY_LENGTH;
        m_dayCounter++;

        /* Сдвигаем начальную позицию восхода на 2 радиана */
        m_dayStartOffset += DAY_SHIFT;
        if (m_dayStartOffset >= 2.0f * 3.14159f)
            m_dayStartOffset -= 2.0f * 3.14159f;

        /* Проверяем, не пора ли запустить эвент */
        if (m_activeEvent == EventType_None && m_dayCounter >= m_eventThreshold) {
            pickNextEventThreshold();
            /* Рандом: 30% шанс что эвент произойдёт при достижении порога */
            if (std::rand() % 100 < 30) {
                /* Выбираем случайный эвент */
                switch (std::rand() % 4) {
                    case 0: triggerEvent(EventType_WhiteNight, 1.0f); break;
                    case 1: triggerEvent(EventType_PolarNight, 1.0f); break;
                    case 2: triggerEvent(EventType_Eclipse, 0.3f); break;
                    case 3: triggerEvent(EventType_Aurora, 0.5f); break;
                }
            }
        }
    }

    /* Обновляем таймер активного эвента */
    updateActiveEvent(dtDays);

    m_state.dayProgress = m_timeOfDay / DAY_LENGTH;

    float dayAngle = m_state.dayProgress * 2.0f * 3.14159f;
    float totalAngle = m_dayStartOffset + dayAngle;
    float hourAngle = totalAngle * 180.0f / 3.14159f - 90.0f;

    calcSun(hourAngle);
    calcMoon(hourAngle);
    calcSkyColor();

    /* Модифицируем состояние если активен эвент */
    applyEventEffects();

    m_state.isDaytime = m_state.sunIntensity > 0.3f;
}

void Cycles::calcSun(float angleDeg) {
    float rad = angleDeg * 3.14159f / 180.0f;

    m_state.sunX = std::cos(rad);
    m_state.sunY = std::sin(rad);
    m_state.sunZ = 0.3f;

    float len = std::sqrt(m_state.sunX*m_state.sunX + m_state.sunY*m_state.sunY + m_state.sunZ*m_state.sunZ);
    if (len > 0.001f) {
        m_state.sunX /= len; m_state.sunY /= len; m_state.sunZ /= len;
    }

    float y = m_state.sunY;

    m_state.sunIntensity = smoothstepC(-0.15f, 0.1f, y);

    if (y > 0.3f) {
        m_state.sunColorR = 1.0f;
        m_state.sunColorG = 0.95f;
        m_state.sunColorB = 0.85f;
    } else if (y > 0.0f) {
        float t = y / 0.3f;
        m_state.sunColorR = 1.0f;
        m_state.sunColorG = 0.45f + 0.50f * t;
        m_state.sunColorB = 0.10f + 0.75f * t;
    } else if (y > -0.15f) {
        float t = (y + 0.15f) / 0.15f;
        m_state.sunColorR = 0.6f + 0.4f * t;
        m_state.sunColorG = 0.10f + 0.35f * t;
        m_state.sunColorB = 0.02f + 0.08f * t;
    } else {
        m_state.sunColorR = 0.0f;
        m_state.sunColorG = 0.0f;
        m_state.sunColorB = 0.0f;
    }

    m_state.moonColorR = 0.80f;
    m_state.moonColorG = 0.82f;
    m_state.moonColorB = 0.95f;

    m_state.ambientIntensity = 0.15f + m_state.sunIntensity * 0.85f;
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
    float y = m_state.sunY;  /* -1..1 */

    /* Плавные переходы с широкими зонами */
    if (y > 0.25f) {
        /* Полный день — голубое небо */
        m_state.skyR = 0.25f; m_state.skyG = 0.45f; m_state.skyB = 0.90f;
        m_state.horizonR = 0.55f; m_state.horizonG = 0.70f; m_state.horizonB = 0.95f;
    } else if (y > 0.10f) {
        /* День → закат: голубой → тёплый */
        float t = (y - 0.10f) / 0.15f;
        m_state.skyR = 0.60f * (1.0f - t) + 0.25f * t;
        m_state.skyG = 0.35f * (1.0f - t) + 0.45f * t;
        m_state.skyB = 0.20f * (1.0f - t) + 0.90f * t;
        m_state.horizonR = 0.80f * (1.0f - t) + 0.55f * t;
        m_state.horizonG = 0.45f * (1.0f - t) + 0.70f * t;
        m_state.horizonB = 0.25f * (1.0f - t) + 0.95f * t;
    } else if (y > 0.0f) {
        /* Закат/рассвет — оранжево-розовый */
        float t = y / 0.10f;
        m_state.skyR = 0.85f * (1.0f - t) + 0.60f * t;
        m_state.skyG = 0.30f * (1.0f - t) + 0.35f * t;
        m_state.skyB = 0.15f * (1.0f - t) + 0.20f * t;
        m_state.horizonR = 0.90f * (1.0f - t) + 0.80f * t;
        m_state.horizonG = 0.40f * (1.0f - t) + 0.45f * t;
        m_state.horizonB = 0.20f * (1.0f - t) + 0.25f * t;
    } else if (y > -0.08f) {
        /* Солнце только за горизонтом — тёмно-красное свечение */
        float t = (y + 0.08f) / 0.08f;
        m_state.skyR = 0.15f * (1.0f - t) + 0.85f * t;
        m_state.skyG = 0.06f * (1.0f - t) + 0.30f * t;
        m_state.skyB = 0.04f * (1.0f - t) + 0.15f * t;
        m_state.horizonR = 0.20f * (1.0f - t) + 0.90f * t;
        m_state.horizonG = 0.08f * (1.0f - t) + 0.40f * t;
        m_state.horizonB = 0.05f * (1.0f - t) + 0.20f * t;
    } else if (y > -0.20f) {
        /* Сумерки — переход от тёмно-красного к ночному */
        float t = (y + 0.20f) / 0.12f;  /* 0 = ночь, 1 = красное свечение */
        m_state.skyR = 0.02f * (1.0f - t) + 0.15f * t;
        m_state.skyG = 0.02f * (1.0f - t) + 0.06f * t;
        m_state.skyB = 0.06f * (1.0f - t) + 0.04f * t;
        m_state.horizonR = 0.04f * (1.0f - t) + 0.20f * t;
        m_state.horizonG = 0.04f * (1.0f - t) + 0.08f * t;
        m_state.horizonB = 0.08f * (1.0f - t) + 0.05f * t;
    } else {
        /* Полная ночь — почти чёрное */
        m_state.skyR = 0.02f; m_state.skyG = 0.02f; m_state.skyB = 0.06f;
        m_state.horizonR = 0.04f; m_state.horizonG = 0.04f; m_state.horizonB = 0.08f;
    }

    /* Туман = цвет горизонта */
    m_state.fogR = m_state.horizonR;
    m_state.fogG = m_state.horizonG;
    m_state.fogB = m_state.horizonB;
}

/* ============================================================
 * Система редких эвентов
 * ============================================================ */

void Cycles::pickNextEventThreshold() {
    /* Рандом: 85-120 дней до следующей проверки */
    m_eventThreshold = m_dayCounter + 85 + (std::rand() % 36);
}

void Cycles::triggerEvent(EventType type, float durationDays) {
    m_activeEvent = type;
    m_eventDuration = durationDays;
    m_eventTimer = 0.0f;

    switch (type) {
        case EventType_WhiteNight:  m_state.eventName = "Белая ночь"; break;
        case EventType_PolarNight:  m_state.eventName = "Полярная ночь"; break;
        case EventType_Eclipse:     m_state.eventName = "Затмение"; break;
        case EventType_Aurora:      m_state.eventName = "Полярное сияние"; break;
        default:                     m_state.eventName = nullptr; break;
    }
}

void Cycles::updateActiveEvent(float dtDays) {
    if (m_activeEvent == EventType_None) {
        m_state.activeEvent = EventType_None;
        m_state.eventName = nullptr;
        m_state.eventProgress = 0.0f;
        return;
    }

    m_eventTimer += dtDays;
    m_state.eventProgress = m_eventTimer / m_eventDuration;
    m_state.activeEvent = m_activeEvent;

    /* Эвент закончился */
    if (m_eventTimer >= m_eventDuration) {
        m_activeEvent = EventType_None;
        m_eventTimer = 0.0f;
        m_eventDuration = 0.0f;
        m_state.eventName = nullptr;
        m_state.eventProgress = 0.0f;
    }
}

void Cycles::applyEventEffects() {
    float p = m_state.eventProgress;  /* 0..1 */

    switch (m_activeEvent) {
        case EventType_WhiteNight: {
            /* Солнце не уходит за горизонт — держим sunY высоким */
            float nightFactor = 1.0f - std::sin(p * 3.14159f);  /* плавная кривая */
            m_state.sunY = std::max(m_state.sunY, 0.3f * nightFactor);
            m_state.sunIntensity = std::max(m_state.sunIntensity, 0.6f * nightFactor);

            /* Небо остаётся светлым, с лёгким фиолетовым оттенком */
            m_state.skyR = std::max(m_state.skyR, 0.15f + 0.25f * nightFactor);
            m_state.skyG = std::max(m_state.skyG, 0.15f + 0.25f * nightFactor);
            m_state.skyB = std::max(m_state.skyB, 0.3f + 0.3f * nightFactor);
            m_state.horizonR = std::max(m_state.horizonR, 0.4f + 0.3f * nightFactor);
            m_state.horizonG = std::max(m_state.horizonG, 0.4f + 0.2f * nightFactor);
            m_state.horizonB = std::max(m_state.horizonB, 0.5f + 0.2f * nightFactor);
            m_state.ambientIntensity = std::max(m_state.ambientIntensity, 0.5f + 0.2f * nightFactor);

            /* Солнце — оранжево-розовое */
            m_state.sunColorR = 1.0f;
            m_state.sunColorG = 0.6f + 0.3f * nightFactor;
            m_state.sunColorB = 0.4f + 0.4f * nightFactor;
            break;
        }

        case EventType_PolarNight: {
            /* Солнце не восходит — держим sunY низким */
            float dayFactor = std::sin(p * 3.14159f);
            m_state.sunY = std::min(m_state.sunY, -0.2f * (1.0f - dayFactor));
            m_state.sunIntensity *= 0.2f;

            /* Небо тёмное */
            m_state.skyR *= 0.3f;
            m_state.skyG *= 0.3f;
            m_state.skyB *= 0.4f;
            m_state.horizonR *= 0.4f;
            m_state.horizonG *= 0.4f;
            m_state.horizonB *= 0.5f;
            m_state.ambientIntensity *= 0.3f;
            break;
        }

        case EventType_Eclipse: {
            /* Затмение — резко темнеет, солнце закрывается */
            float eclipseMid = std::sin(p * 3.14159f);  /* 0 → 1 → 0 */
            m_state.sunIntensity *= (1.0f - eclipseMid * 0.9f);
            m_state.sunColorR = 1.0f - eclipseMid * 0.5f;
            m_state.sunColorG = 0.95f - eclipseMid * 0.8f;
            m_state.sunColorB = 0.85f - eclipseMid * 0.7f;

            /* Небо темнеет до сумеречного */
            m_state.skyR = m_state.skyR * (1.0f - eclipseMid * 0.7f);
            m_state.skyG = m_state.skyG * (1.0f - eclipseMid * 0.7f);
            m_state.skyB = m_state.skyB * (1.0f - eclipseMid * 0.5f);
            m_state.ambientIntensity = 0.15f + m_state.ambientIntensity * (1.0f - eclipseMid * 0.5f);
            break;
        }

        case EventType_Aurora: {
            /* Полярное сияние — зелёно-фиолетовые оттенки в небе */
            float auroraIntensity = std::sin(p * 3.14159f);
            float shimmer = std::sin(p * 20.0f) * 0.5f + 0.5f;  /* мерцание */

            /* Зелёно-фиолетовый оттенок неба */
            m_state.skyR += 0.1f * auroraIntensity * (1.0f - shimmer);
            m_state.skyG += 0.3f * auroraIntensity * shimmer;
            m_state.skyB += 0.2f * auroraIntensity * (0.5f + 0.5f * shimmer);
            m_state.horizonR += 0.05f * auroraIntensity;
            m_state.horizonG += 0.25f * auroraIntensity * shimmer;
            m_state.horizonB += 0.15f * auroraIntensity;
            m_state.fogR = m_state.skyR;
            m_state.fogG = m_state.skyG;
            m_state.fogB = m_state.skyB;
            break;
        }

        default:
            break;
    }
}

} // namespace be::void_::physics
