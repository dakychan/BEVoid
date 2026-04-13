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

vec3 hash3(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
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

float hash2(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }

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

float cloudFBM(vec3 p) {
    float v = noise3D(p) * 0.625;
    v += noise3D(p * 2.07) * 0.25;
    v += noise3D(p * 4.14) * 0.1;
    v += noise3D(p * 8.28) * 0.025;
    return v;
}

vec3 renderClouds(vec3 dir, float sunElev) {
    float horizonFade = smoothstep(0.03, 0.15, dir.y);
    if (horizonFade < 0.01) return vec3(0.0);

    float dayF = smoothstep(-0.1, 0.25, sunElev);
    vec2 wind = normalize(vec2(1.0, 0.3)) * uTime * 0.2;

    vec3 accum = vec3(0.0);
    float trans = 1.0;

    float cBot = uCamWorldPos.y + 120.0;
    float cTop = uCamWorldPos.y + 280.0;

    float tIn = max((cBot - uCamWorldPos.y) / max(dir.y, 0.001), 0.0);
    float tOut = (cTop - uCamWorldPos.y) / max(dir.y, 0.001);
    if (tOut <= 0.0 || tIn >= tOut) return vec3(0.0);

    int STEPS = 8;
    float dt = (tOut - tIn) / float(STEPS);

    vec3 topDay = vec3(0.97, 0.96, 0.94);
    vec3 botDay = vec3(0.60, 0.62, 0.68);
    vec3 topSet = uSunColor * 1.1 + vec3(0.04, 0.01, 0.0);
    vec3 botSet = uSunColor * 0.3;
    vec3 topNight = vec3(0.04, 0.04, 0.08) + uMoonColor * 0.04;
    vec3 botNight = vec3(0.02, 0.02, 0.04);

    vec3 cTopCol, cBotCol;
    if (sunElev > 0.2) {
        cTopCol = mix(topDay, uSunColor, 0.12);
        cBotCol = botDay;
    } else if (sunElev > 0.0) {
        float t = sunElev / 0.2;
        cTopCol = mix(topSet, topDay, t);
        cBotCol = mix(botSet, botDay, t);
    } else if (sunElev > -0.1) {
        float t = (sunElev + 0.1) / 0.1;
        cTopCol = mix(topNight, topSet, t);
        cBotCol = mix(botNight, botSet, t);
    } else {
        cTopCol = topNight;
        cBotCol = botNight;
    }

    for (int i = 0; i < STEPS; i++) {
        if (trans < 0.02) break;

        float t = tIn + dt * (float(i) + 0.5);
        vec3 wp = uCamWorldPos + dir * t;
        float hFrac = clamp((wp.y - cBot) / (cTop - cBot), 0.0, 1.0);

        float prof = smoothstep(0.0, 0.08, hFrac) * smoothstep(1.0, 0.4, hFrac);

        vec3 np = vec3(wp.xz * 0.0015 + wind * 0.1, 1.7);
        float n = cloudFBM(np);

        float thresh = mix(0.22, 0.12, dayF);
        float dens = max(n - thresh, 0.0) / max(1.0 - thresh, 0.01);
        dens *= prof;

        if (dens < 0.002) continue;

        float hGrad = smoothstep(0.2, 0.8, hFrac);
        vec3 col = mix(cBotCol, cTopCol, hGrad);

        float edge = 1.0 - smoothstep(0.0, 0.15, dens);
        col += edge * cTopCol * 0.12 * dayF;

        float a = dens * dt * 0.012;
        accum += col * a * trans;
        trans *= (1.0 - a);
    }

    return accum * horizonFade;
}

float starHash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(0.1031, 0.1030, 0.0973));
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 renderStars(vec3 dir, float sunElev) {
    if (dir.y < 0.05) return vec3(0.0);
    if (sunElev > -0.15) return vec3(0.0);

    float nightF = smoothstep(-0.15, -0.35, sunElev);
    if (nightF < 0.01) return vec3(0.0);

    float cycleVal = sin(uTime * 0.008) * 0.5 + 0.5;
    float density = mix(0.004, 0.0002, pow(max(cycleVal, 0.0), 8.0));

    vec2 gridUV = dir.xy / dir.z * 180.0;
    vec2 cell = floor(gridUV);
    vec2 cellCenter = (cell + 0.5);
    vec2 offset = gridUV - cellCenter;

    float h = starHash(cell);
    if (h > density) return vec3(0.0);

    float d = length(offset);
    float brightness = smoothstep(0.45, 0.0, d);
    brightness *= nightF * (0.3 + 0.7 * h);

    return vec3(brightness * 0.9, brightness * 0.92, brightness);
}

void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;

    float t = pow(max(h, 0.0), 0.45);
    vec3 sky = mix(uHorizonColor, uTopColor, t);

    float sdot = dot(dir, uSunDir);

    float sunGlow = smoothstep(-0.3, 0.1, uSunElevation);
    sky = mix(sky, uSunColor, pow(max(sdot, 0.0), 4.0) * 0.3 * sunGlow);
    sky += uSunColor * smoothstep(0.96, 1.0, max(sdot, 0.0)) * 3.0 * sunGlow;
    sky += uSunColor * pow(max(sdot, 0.0), 16.0) * 0.5 * sunGlow;

    float belowGlow = smoothstep(0.0, -0.3, uSunElevation);
    sky += uSunColor * pow(max(sdot, 0.0), 2.0) * 0.1 * belowGlow;

    float moonF = smoothstep(0.0, -0.25, uSunElevation);
    float mdot = dot(dir, uMoonDir);
    if (moonF > 0.01) {
        float ma = acos(clamp(mdot, -1.0, 1.0));
        vec2 muv = dir.xy / max(mdot, 0.001);
        float surf = smoothstep(0.35, 0.65, fbm2(muv * 8.0));
        float md = (1.0 - smoothstep(0.024, 0.033, ma)) * surf;
        sky += uMoonColor * 1.2 * (md * 1.5 + pow(max(mdot, 0.0), 32.0) * 0.08) * moonF;
    }

    sky += renderStars(dir, uSunElevation);

    vec3 clouds = renderClouds(dir, uSunElevation);
    float cloudLen = length(clouds);
    sky = mix(sky, clouds, smoothstep(0.0, 0.3, cloudLen));

    fragColor = vec4(sky, 1.0);
}
