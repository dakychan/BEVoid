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

void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;
    
    // Градиент неба
    float t = pow(max(h, 0.0), 0.45);
    vec3 sky = mix(uHorizonColor, uTopColor, t);

    // Ореол солнца — мягкое свечение вокруг (влияет на небо рядом)
    float sdot = dot(dir, uSunDir);
    if (uSunElevation > -0.05f) {
        // Широкое ореол-свечение (влияет на цвет неба вокруг солнца)
        float sunHalo = pow(max(sdot, 0.0), 4.0) * 0.3;
        float sunGlow = pow(max(sdot, 0.0), 16.0) * 0.5;
        float sunDisk = smoothstep(0.96, 1.0, max(sdot, 0.0));
        // Ореол микширует цвет неба с цветом солнца
        sky = mix(sky, uSunColor, sunHalo);
        // Яркий диск + свечение поверх
        sky += uSunColor * (sunDisk * 3.0 + sunGlow);
    }

    // Луна — только диск + свечение, БЕЗ ореола на небе
    float mdot = dot(dir, uMoonDir);
    if (uSunElevation < 0.1f) {
        float moonDisk = smoothstep(0.96, 1.0, max(mdot, 0.0));
        float moonGlow = pow(max(mdot, 0.0), 16.0) * 0.3;
        float nightFade = clamp((0.1f - uSunElevation) / 0.15f, 0.0, 1.0);
        sky += uMoonColor * (moonDisk * 2.0 + moonGlow) * nightFade;
    }

    // Простые облака
    if (h > 0.05) {
        float cloud = sin(dir.x * 4.0 + uTime * 0.05) * cos(dir.z * 3.0 + uTime * 0.03);
        float fade = smoothstep(0.05, 0.2, h);
        sky = mix(sky, vec3(0.85, 0.88, 0.92), max(cloud, 0.0) * fade * 0.4);
    }

    fragColor = vec4(sky, 1.0);
}
