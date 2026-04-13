#version 330 core
in vec3 vDir;
in vec3 vWorldPos;
out vec4 fragColor;
uniform float uTime;
uniform vec3 uTopColor;
uniform vec3 uHorizonColor;
uniform vec3 uSunColor;
uniform vec3 uSunDir;
uniform float uSunElevation;
uniform vec3 uMoonColor;
uniform vec3 uMoonDir;
uniform vec3 uCamWorldPos;

/* ============================================================
 * 3D Value Noise
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

vec3 renderClouds(vec3 dir, float sunElev) {
    // Облака только над горизонтом
    float horizonFade = smoothstep(0.02, 0.08, dir.y);
    if (horizonFade < 0.01) return vec3(0.0);

    // Ветер — безветренная погода
    vec2 wind = normalize(vec2(1.0, 0.3)) * uTime * 0.3;

    vec3 accum = vec3(0.0);
    float remainingTrans = 1.0;

    // Облака на одной высоте
    float cloudBottom = uCamWorldPos.y + 100.0;
    float cloudTop = uCamWorldPos.y + 250.0;
    float thickness = cloudTop - cloudBottom;

    float tEntry = max((cloudBottom - uCamWorldPos.y) / max(dir.y, 0.001), 0.0);
    float tExit = (cloudTop - uCamWorldPos.y) / max(dir.y, 0.001);
    if (tExit <= 0.0 || tEntry >= tExit) return vec3(0.0);

    int steps = 6;
    float stepSize = (tExit - tEntry) / float(steps);

    for (int i = 0; i < steps; i++) {
        if (remainingTrans < 0.03) break;

        float t = tEntry + stepSize * (float(i) + 0.5);
        vec3 worldPos = uCamWorldPos + dir * t;

        // UV для шума
        vec2 uv = dir.xz / max(dir.y, 0.01);
        uv *= 0.5;
        uv += wind * 0.15;

        // СЛОЙ 1: Где ОБЛАКА
        float cover = noise3D(vec3(uv * 0.4, 1.7));
        cover = cover * 0.5 + 0.5;

        // Мягкий порог — отдельные облака
        float cloudMask = smoothstep(0.55, 0.75, cover);
        if (cloudMask < 0.01) continue;

        // СЛОЙ 2: Форма
        float shape = noise3D(vec3(uv * 1.2, 2.3));
        shape = shape * 0.5 + 0.5;

        // СЛОЙ 3: Пух
        float fluff = noise3D(vec3(uv * 3.0, 3.7));
        fluff = fluff * 0.5 + 0.5;

        // Рандомная высота внутри слоя
        float hNoise = noise3D(vec3(uv * 0.6, 12.0));
        hNoise = hNoise * 0.5 + 0.5;
        float centerY = cloudBottom + thickness * (0.2 + hNoise * 0.6);
        float cThick = thickness * (0.15 + hNoise * 0.25);

        float ny = clamp((worldPos.y - centerY) / max(cThick * 0.5, 1.0), -1.0, 1.0);
        ny = abs(ny);  // 0 в центре, 1 на краях

        // Комбинируем
        float dens = cloudMask * (shape * 0.6 + fluff * 0.4);
        dens *= max(1.0 - ny * 0.6, 0.0);  // максимум в центре

        if (dens < 0.03) continue;

        // Свет
        float sunDot = max(dot(dir, uSunDir), 0.0);
        float light = 0.9 + 0.1 * sunDot;
        vec3 cloudCol = vec3(1.0, 1.0, 1.0) * light;

        float alpha = dens * stepSize * 0.12;
        accum += cloudCol * alpha * remainingTrans;
        remainingTrans *= (1.0 - alpha);
    }

    return accum * horizonFade;
}

/* ============================================================ */
void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;

    // Градиент неба (атмосфера)
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

    // Облака — часть атмосферы, гипер-высота
    vec3 clouds = renderClouds(dir, uSunElevation);
    sky = mix(sky, clouds, clamp(length(clouds), 0.0, 0.7));

    fragColor = vec4(sky, 1.0);
}
