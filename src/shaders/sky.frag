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

/* Простой 2D noise для текстуры луны */
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453);
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p) {
    float val = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; i++) {
        val += noise(p) * amp;
        p *= 2.0;
        amp *= 0.5;
    }
    return val;
}

void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;

    // Градиент неба
    float t = pow(max(h, 0.0), 0.45);
    vec3 sky = mix(uHorizonColor, uTopColor, t);

    // Ореол солнца — мягкое свечение вокруг
    float sdot = dot(dir, uSunDir);
    if (uSunElevation > -0.05f) {
        float sunHalo = pow(max(sdot, 0.0), 4.0) * 0.3;
        float sunGlow = pow(max(sdot, 0.0), 16.0) * 0.5;
        float sunDisk = smoothstep(0.96, 1.0, max(sdot, 0.0));
        sky = mix(sky, uSunColor, sunHalo);
        sky += uSunColor * (sunDisk * 3.0 + sunGlow);
    }

    // Луна — с текстурой кратеров и мягким глоу
    float mdot = dot(dir, uMoonDir);
    if (uSunElevation < 0.15f) {
        float moonAngle = acos(clamp(mdot, -1.0, 1.0));
        float moonRadius = 0.03;  // радиус лунного диска (~1.7°)

        // UV-координаты на поверхности луны
        vec2 moonUV = dir.xy / max(mdot, 0.001);

        // Процедурная текстура кратеров
        float surface = fbm(moonUV * 8.0);
        surface = smoothstep(0.35, 0.65, surface);

        // Диск луны с мягкими краями
        float moonDisk = 1.0 - smoothstep(moonRadius * 0.8, moonRadius * 1.1, moonAngle);
        moonDisk *= surface;

        // Мягкий глоу — маленький и аккуратный
        float moonGlow = pow(max(mdot, 0.0), 32.0) * 0.08;
        float glowRing = pow(max(mdot, 0.0), 20.0) * 0.04;

        // Появление луны — плавное, когда солнце уходит
        float nightFade = clamp((0.15f - uSunElevation) / 0.2f, 0.0, 1.0);

        vec3 moonBright = uMoonColor * 1.2;
        sky += moonBright * (moonDisk * 1.5 + moonGlow + glowRing) * nightFade;
    }

    // Простые облака
    if (h > 0.05) {
        float cloud = sin(dir.x * 4.0 + uTime * 0.05) * cos(dir.z * 3.0 + uTime * 0.03);
        float fade = smoothstep(0.05, 0.2, h);
        sky = mix(sky, vec3(0.85, 0.88, 0.92), max(cloud, 0.0) * fade * 0.4);
    }

    fragColor = vec4(sky, 1.0);
}
