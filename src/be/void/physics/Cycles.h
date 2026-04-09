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
#include <string>

namespace be::void_::physics {

/* Тип активного эвента */
enum EventType {
    EventType_None = 0,
    EventType_WhiteNight,       /* Солнце не уходит за горизонт — светло всю ночь */
    EventType_PolarNight,       /* Солнце не восходит — темнота весь день */
    EventType_Eclipse,          /* Затмение — резко темнеет днём */
    EventType_Aurora,           /* Полярное сияние — яркие цвета в небе */
};

struct SunMoonState {
    /* Солнце */
    float sunX, sunY, sunZ;    /* направление на солнце (нормализованное) */
    float sunIntensity;         /* 0..1 */
    float sunColorR, sunColorG, sunColorB;

    /* Луна */
    float moonX, moonY, moonZ;
    float moonPhase;            /* 0..1 (полная→новая) */
    float moonColorR, moonColorG, moonColorB;

    /* Небо */
    float skyR, skyG, skyB;     /* цвет неба (зенит) */
    float horizonR, horizonG, horizonB; /* цвет горизонта */
    float fogR, fogG, fogB;     /* цвет тумана */
    float ambientIntensity;     /* 0..1 */

    /* Время */
    float dayProgress;          /* 0..1 (полночь→полночь) */
    bool  isDaytime;

    /* Эвент */
    EventType activeEvent;
    const char* eventName;      /* nullptr если нет активного эвента */
    float eventProgress;        /* 0..1 прогресс текущего эвента */
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
    float m_timeOfDay = 17.0f * 60.0f; /* старт в 17:00 — ближе к закату (1020 мин) */
    SunMoonState m_state;

    /* Фазы луны (цикл ~29.5 дней) */
    float m_moonCycle = 29.5f * DAY_LENGTH;

    /* Смещение стартовой позиции солнца (сдвиг на 2 радиана за каждый новый день) */
    float m_dayStartOffset = 0.0f;
    static constexpr float DAY_SHIFT = 2.0f;  /* сдвиг восхода за каждый цикл дня */

    /* --- Система редких эвентов --- */
    uint32_t m_dayCounter = 0;
    uint32_t m_eventThreshold = 0;   /* через сколько дней следующий эвент */
    EventType m_activeEvent = EventType_None;
    float m_eventDuration = 0.0f;    /* сколько длится эвент в днях */
    float m_eventTimer = 0.0f;       /* сколько эвент уже идёт */

    void pickNextEventThreshold();
    void triggerEvent(EventType type, float durationDays);
    void updateActiveEvent(float dtDays);
    void applyEventEffects();
};

} // namespace be::void_::physics

#endif // BEVOID_CYCLES_H
