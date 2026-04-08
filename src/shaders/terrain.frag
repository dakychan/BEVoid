#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in float Height;

out vec4 fragColor;

uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uSkyColor;
uniform float uAmbient;
uniform vec3 uCamPos;
uniform float uTime;

float hash(vec3 p){
    p = fract(p * vec3(.1031,.1030,.0973));
    p += dot(p, p + 33.33);
    return fract((p.x + p.y) * p.z);
}

float noise(vec3 p){
    vec3 i = floor(p), f = fract(p);
    f = f*f*(3.0 - 2.0*f);
    return mix(mix(mix(hash(i), hash(i+vec3(1,0,0)), f.x),
                   mix(hash(i+vec3(0,1,0)), hash(i+vec3(1,1,0)), f.x), f.y),
               mix(mix(hash(i+vec3(0,0,1)), hash(i+vec3(1,0,1)), f.x),
                   mix(hash(i+vec3(0,1,1)), hash(i+vec3(1,1,1)), f.x), f.y), f.z);
}

float fbm(vec3 p){
    float v = 0.0, a = 0.5;
    for(int i = 0; i < 5; i++){ v += a * noise(p); p *= 2.01; a *= 0.5; }
    return v;
}

void main() {
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, uSunDir), 0.0);
    vec3 lighting = uAmbient * uSkyColor + diff * uSunColor;
    vec3 result = Color * lighting;

    // Туман по расстоянию — агрессивный, полностью скрывает границы чанков
    float dist = length(FragPos - uCamPos);
    // Начинается на 40м, полный на 140м
    float fogFactor = clamp((dist - 40.0) / 100.0, 0.0, 1.0);
    fogFactor = fogFactor * fogFactor * (3.0 - 2.0 * fogFactor); // smoothstep

    // Цвет неба для тумана
    vec3 camDir = normalize(FragPos - uCamPos);
    float h = camDir.y;
    vec3 zenith = vec3(0.25, 0.45, 0.90);
    vec3 horizon = mix(vec3(0.65, 0.78, 0.92), uSkyColor, 0.5);
    float t = pow(max(h, 0.0), 0.4);
    vec3 skyGrad = mix(horizon, zenith, t);

    result = mix(result, skyGrad, fogFactor);

    fragColor = vec4(result, 1.0);
}
