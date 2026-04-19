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
 * be.void.physics.metrics — Unit system
 *
 * Система единиц измерения: метры, километры, сантиметры.
 * Все расчеты в метрах, конвертация для удобства.
 */

#ifndef BE_VOID_PHYSICS_METRICS_UNITS_H
#define BE_VOID_PHYSICS_METRICS_UNITS_H

namespace be::void_::physics::metrics {

// Базовая единица — метр
using Meters = float;

// Константы конвертации
constexpr float METERS_PER_KM = 1000.0f;
constexpr float CM_PER_METER = 100.0f;
constexpr float MM_PER_METER = 1000.0f;

// Конвертация из километров в метры
inline constexpr Meters toMeters(float kilometers) {
    return kilometers * METERS_PER_KM;
}

// Конвертация из метров в километры
inline constexpr float toKilometers(Meters meters) {
    return meters / METERS_PER_KM;
}

// Конвертация из сантиметров в метры
inline constexpr Meters fromCentimeters(float centimeters) {
    return centimeters / CM_PER_METER;
}

// Конвертация из метров в сантиметры
inline constexpr float toCentimeters(Meters meters) {
    return meters * CM_PER_METER;
}

// Конвертация из миллиметров в метры
inline constexpr Meters fromMillimeters(float millimeters) {
    return millimeters / MM_PER_METER;
}

// Конвертация из метров в миллиметры
inline constexpr float toMillimeters(Meters meters) {
    return meters * MM_PER_METER;
}

// Ограничения для биомов (не миллионы метров!)
constexpr Meters MAX_BIOME_SIZE = 2000.0f;      // 2 км максимум
constexpr Meters MIN_BIOME_SIZE = 500.0f;       // 500 м минимум
constexpr Meters CHUNK_SIZE_METERS = 32.0f;     // Размер чанка в метрах

} // namespace be::void_::physics::metrics

#endif // BE_VOID_PHYSICS_METRICS_UNITS_H
