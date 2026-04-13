#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out float Height;

uniform mat4 uView;
uniform mat4 uProj;
uniform float uTime;

void main() {
    vec3 pos = aPos;

    // Волны на воде — для вершин ниже WATER_LEVEL (10.0)
    float WATER_LEVEL = 10.0;
    if (pos.y < WATER_LEVEL + 2.0) {
        float depth = WATER_LEVEL - pos.y;
        float waveAmount = smoothstep(5.0, -2.0, depth);  // максимум у поверхности

        // Многослойные волны
        float wave1 = sin(pos.x * 0.5 + uTime * 1.2) * cos(pos.z * 0.3 + uTime * 0.8);
        float wave2 = sin(pos.x * 0.8 - uTime * 0.9) * sin(pos.z * 0.6 + uTime * 1.1);
        float wave3 = sin(pos.x * 0.3 + pos.z * 0.4 + uTime * 0.5) * 0.5;

        pos.y += (wave1 * 0.6 + wave2 * 0.3 + wave3) * waveAmount * 0.8;
    }

    FragPos = pos;
    Normal = aNormal;
    Color = aColor;
    Height = pos.y;
    gl_Position = uProj * uView * vec4(pos, 1.0);
}
