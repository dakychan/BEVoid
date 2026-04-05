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
 * Цикл дня/ночи: солнце, луна, закат, рассвет.
 * Основано на реальном времени игрока (часовой пояс).
 */

#ifndef BEVOID_CYCLES_H
#define BEVOID_CYCLES_H

#include <cstdint>
#include <cmath>

namespace be::void_::physics {

struct SunMoonState {
    /* Солнце */
    float sunX, sunY, sunZ;    /* направление на солнце (нормализованное) */
    float sunIntensity;         /* 0..1 */
    float sunColorR, sunColorG, sunColorB;

    /* Луна */
    float moonX, moonY, moonZ;
    float moonPhase;            /* 0..1 (полная→новая) */

    /* Небо */
    float skyR, skyG, skyB;     /* цвет неба */
    float fogR, fogG, fogB;     /* цвет тумана */
    float ambientIntensity;     /* 0..1 */

    /* Время */
    float dayProgress;          /* 0..1 (полночь→полночь) */
    bool  isDaytime;
};

class Cycles {
public:
    Cycles();
    ~Cycles();

    /* Настроить часовой пояс (часы от UTC) */
    void setTimezone(float utcOffset);

    /* Обновить — вызвать каждый кадр */
    void update(float deltaTime);

    /* Получить текущее состояние */
    const SunMoonState& getState() const { return m_state; }

    /* День в секундах (24 мин = 1 игровой день) */
    static constexpr float DAY_LENGTH = 24.0f * 60.0f; /* 1440 сек = 24 мин */

private:
    /* Рассчитать положение солнца по времени суток */
    void calcSun(float hourAngle);

    /* Рассчитать положение луны */
    void calcMoon(float hourAngle);

    /* Цвет неба по положению солнца */
    void calcSkyColor();

    float m_timezone = 3.0f; /* UTC+3 по умолчанию (Москва) */
    float m_timeOfDay = 8.0f * 60.0f; /* старт в 8:00 утра (480 мин) */
    SunMoonState m_state;

    /* Фазы луны (цикл ~29.5 дней) */
    float m_moonCycle = 29.5f * DAY_LENGTH;
};

} // namespace be::void_::physics

#endif // BEVOID_CYCLES_H
