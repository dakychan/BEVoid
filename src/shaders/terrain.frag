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

    // Туман по расстоянию
    float dist = distance(FragPos, uCamPos);
    float fog = clamp(dist / 250.0, 0.0, 1.0);
    fog = fog * fog;

    // Градиент неба
    vec3 camDir = normalize(FragPos - uCamPos);
    float h = camDir.y;
    float t = pow(max(h, 0.0), 0.45);
    vec3 skyGrad = mix(uSkyColor, vec3(uSkyColor.r*0.5, uSkyColor.g*0.7, uSkyColor.b*1.1), t);

    // Солнце
    float sdot = max(dot(camDir, uSunDir), 0.0);
    vec3 sunCol = uSunColor * (pow(sdot, 300.0) * 5.0 + pow(sdot, 8.0) * 0.35);
    skyGrad += sunCol;

    // Облака — яркие, заметные
    if(h > 0.01){
        vec3 cp = vec3(camDir.xz * 5.0, uTime * 0.025);
        float c = fbm(cp);
        c = smoothstep(0.38, 0.68, c);
        float fade = smoothstep(0.01, 0.12, h);
        vec3 cc = vec3(0.92, 0.93, 0.96) * (0.55 + 0.45 * sdot);
        skyGrad = mix(skyGrad, cc, c * fade * 0.85);
    }

    // Микс террейн → небо
    result = mix(result, skyGrad, fog);

    fragColor = vec4(result, 1.0);
}
