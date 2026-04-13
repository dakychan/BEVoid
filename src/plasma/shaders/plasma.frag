#version 330 core
in vec3 vDir;
out vec4 fragColor;
uniform float uTime;
uniform vec3 uTopColor;
uniform vec3 uHorizonColor;
uniform vec3 uSunColor;
uniform vec3 uSunDir;
uniform float uSunElevation;
uniform vec3 uMoonColor;
uniform vec3 uMoonDir;

/* ============================================================
 * Быстрый 3D noise
 * ============================================================ */
vec3 hash3(vec3 p) {
    p = vec3(
        dot(p, vec3(127.1, 311.7, 74.7)),
        dot(p, vec3(269.5, 183.3, 246.1)),
        dot(p, vec3(113.5, 271.9, 124.6))
    );
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float noise3D(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(mix(mix(dot(hash3(i + vec3(0,0,0)), f - vec3(0,0,0)),
                       dot(hash3(i + vec3(1,0,0)), f - vec3(1,0,0)), u.x),
                   mix(dot(hash3(i + vec3(0,1,0)), f - vec3(0,1,0)),
                       dot(hash3(i + vec3(1,1,0)), f - vec3(1,1,0)), u.x), u.y),
               mix(mix(dot(hash3(i + vec3(0,0,1)), f - vec3(0,0,1)),
                       dot(hash3(i + vec3(1,0,1)), f - vec3(1,0,1)), u.x),
                   mix(dot(hash3(i + vec3(0,1,1)), f - vec3(0,1,1)),
                       dot(hash3(i + vec3(1,1,1)), f - vec3(1,1,1)), u.x), u.y), u.z);
}

float hash2(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise2D(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    return mix(mix(hash2(i), hash2(i + vec2(1,0)), f.x),
               mix(hash2(i + vec2(0,1)), hash2(i + vec2(1,1)), f.x), f.y);
}

float fbm2(vec2 p) {
    float v = 0.0, a = 0.5;
    for (int i = 0; i < 3; i++) { v += noise2D(p) * a; p *= 2.0; a *= 0.5; }
    return v;
}

/* ============================================================
 * 3D ОБЛАКА — полноценный raymarching через объём слоя
 * Как в sfsim: ray_sphere intersection + ray march через толщу
 * ============================================================ */

/* Пересечение луча со сферой (центр 0,0,0 радиус r) */
vec2 raySphere(float r, vec3 origin, vec3 dir) {
    float b = dot(origin, dir);
    float c = dot(origin, origin) - r * r;
    float disc = b * b - c;
    if (disc < 0.0) return vec2(-1.0);
    disc = sqrt(disc);
    return vec2(-b - disc, -b + disc);
}

/* Vertical profile — плотность по высоте (как в sfsim cloud-profile.glsl) */
float cloudProfile(float dist) {
    float rising = clamp(dist * 8.0, 0.0, 1.0);
    float plateau = mix(1.0, 0.4, dist * 1.6 - 0.2);
    float falling = mix(0.4, 0.0, dist * 4.0 - 3.0);
    return clamp(min(rising, min(plateau, falling)), 0.0, 1.0);
}

/* Плотность облака в точке — cloud cover + detail noise */
float cloudDensity(vec3 worldPos, vec2 wind, float coverFreq, float detailFreq, float coverSeed, float detailSeed) {
    // Cloud cover — определяет ГДЕ облака
    vec2 coverUV = worldPos.xz * coverFreq + wind * 0.4;
    float cover = noise3D(vec3(coverUV, coverSeed));
    cover = cover * 0.5 + 0.5;

    if (cover < 0.35) return 0.0;  // ранний выход — нет облака

    // Detail noise — форма облака
    vec2 detailUV = worldPos.xz * detailFreq + wind * 0.7;
    float detail = noise3D(vec3(detailUV, detailSeed));
    detail = detail * 0.5 + 0.5;

    return cover * detail;
}

vec3 renderClouds3D(vec3 camDir, float sunElev) {
    if (camDir.y < 0.01) return vec3(0.0);

    // Ветер
    vec2 wind = normalize(vec2(1.0, 0.3)) * uTime * 1.8;

    vec3 accum = vec3(0.0);
    float remainingTrans = 1.0;

    // Цвет облаков
    float dayFactor = clamp(sunElev * 2.0 + 0.5, 0.0, 1.0);
    vec3 dayCol = vec3(0.95, 0.94, 0.92);
    vec3 sunsetCol = vec3(1.0, 0.75, 0.55);
    vec3 nightCol = vec3(0.1, 0.11, 0.16);

    vec3 cloudCol;
    if (dayFactor > 0.5) cloudCol = dayCol;
    else if (dayFactor > 0.15) cloudCol = mix(sunsetCol, dayCol, (dayFactor - 0.15) / 0.35);
    else cloudCol = mix(nightCol, sunsetCol, dayFactor / 0.15);

    // Камера в центре сферы, луч идёт от 0 наружу.
    // Пересечение с оболочкой [r_bottom, r_top] = просто [r_bottom, r_top]
    
    // === СЛОЙ 1: Кучевые — 150..190м ===
    float r1_bottom = 150.0, r1_top = 190.0;
    float tEntry1 = r1_bottom;
    float tExit1 = r1_top;
    float thickness1 = tExit1 - tEntry1;
    int steps1 = 5;
    float stepSize1 = thickness1 / float(steps1);

    for (int i = 0; i < steps1; i++) {
        if (remainingTrans < 0.05) break;

        float t = tEntry1 + stepSize1 * (float(i) + 0.5);
        vec3 worldPos = camDir * t;

        // Vertical profile
        float heightFromBottom = t - r1_bottom;
        float profDist = heightFromBottom / thickness1;
        float prof = cloudProfile(profDist);

        // 3D плотность — cloud cover + detail
        float dens = cloudDensity(worldPos, wind, 0.005, 0.018, 1.7, 2.3);
        if (dens <= 0.0) continue;

        dens *= prof;
        dens = smoothstep(0.2, 0.55, dens);

        float alpha = dens * stepSize1 * 0.1;
        accum += cloudCol * alpha * remainingTrans;
        remainingTrans *= (1.0 - alpha);
    }

    // === СЛОЙ 2: Перистые — 250..300м ===
    if (remainingTrans > 0.1) {
        float r2_bottom = 250.0, r2_top = 300.0;
        float tEntry2 = r2_bottom;
        float tExit2 = r2_top;
        float thickness2 = tExit2 - tEntry2;
        int steps2 = 4;
        float stepSize2 = thickness2 / float(steps2);

        vec3 thinCol = cloudCol * 0.85;

        for (int i = 0; i < steps2; i++) {
            if (remainingTrans < 0.05) break;

            float t = tEntry2 + stepSize2 * (float(i) + 0.5);
            vec3 worldPos = camDir * t;

            float heightFromBottom = t - r2_bottom;
            float profDist = heightFromBottom / thickness2;
            float prof = cloudProfile(profDist);
            if (prof <= 0.0) continue;

            float dens = cloudDensity(worldPos, wind, 0.003, 0.010, 4.1, 5.2);
            if (dens <= 0.0) continue;

            dens *= prof;
            dens = smoothstep(0.12, 0.4, dens);

            float alpha = dens * stepSize2 * 0.05;
            accum += thinCol * alpha * remainingTrans;
            remainingTrans *= (1.0 - alpha);
        }
    }

    return accum;
}

/* ============================================================ */
void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;

    float t = pow(max(h, 0.0), 0.45);
    vec3 sky = mix(uHorizonColor, uTopColor, t);

    // Солнце
    float sdot = dot(dir, uSunDir);
    if (uSunElevation > -0.05) {
        sky = mix(sky, uSunColor, pow(max(sdot, 0.0), 4.0) * 0.3);
        sky += uSunColor * (smoothstep(0.96, 1.0, max(sdot, 0.0)) * 3.0 + pow(max(sdot, 0.0), 16.0) * 0.5);
    }

    // Луна
    float mdot = dot(dir, uMoonDir);
    if (uSunElevation < 0.15) {
        float ma = acos(clamp(mdot, -1.0, 1.0));
        vec2 muv = dir.xy / max(mdot, 0.001);
        float surf = smoothstep(0.35, 0.65, fbm2(muv * 8.0));
        float md = (1.0 - smoothstep(0.024, 0.033, ma)) * surf;
        float nf = clamp((0.15 - uSunElevation) / 0.2, 0.0, 1.0);
        sky += uMoonColor * 1.2 * (md * 1.5 + pow(max(mdot, 0.0), 32.0) * 0.08) * nf;
    }

    // 3D облака
    sky += renderClouds3D(dir, uSunElevation);

    fragColor = vec4(sky, 1.0);
}
