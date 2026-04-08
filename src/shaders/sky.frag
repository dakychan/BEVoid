#version 330 core
in vec3 vDir;
out vec4 fragColor;
uniform float uTime;
uniform vec3 uTopColor;
uniform vec3 uHorizonColor;
uniform vec3 uSunColor;
uniform vec3 uSunDir;
uniform float uSunElevation;

void main() {
    vec3 dir = normalize(vDir);
    float h = dir.y;
    
    // Градиент неба
    float t = pow(max(h, 0.0), 0.45);
    vec3 sky = mix(uHorizonColor, uTopColor, t);

    // Солнце — яркий диск + свечение
    float sdot = dot(dir, uSunDir);
    float sunDisk = smoothstep(0.96, 1.0, sdot);
    float sunGlow = pow(max(sdot, 0.0), 16.0) * 0.5;
    sky += uSunColor * (sunDisk * 3.0 + sunGlow);

    // Простые облака — один sin/cos слой
    if (h > 0.05) {
        float cloud = sin(dir.x * 4.0 + uTime * 0.05) * cos(dir.z * 3.0 + uTime * 0.03);
        float fade = smoothstep(0.05, 0.2, h);
        sky = mix(sky, vec3(0.85, 0.88, 0.92), max(cloud, 0.0) * fade * 0.4);
    }

    fragColor = vec4(sky, 1.0);
}
