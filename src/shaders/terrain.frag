#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in float Height;

out vec4 FragColor;

uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uSkyColor;
uniform float uAmbient;

void main() {
    // Нормализуем нормаль
    vec3 norm = normalize(Normal);
    
    // Диффузное освещение от солнца
    float diff = max(dot(norm, uSunDir), 0.0);
    vec3 diffuse = diff * uSunColor;
    
    // Ambient освещение
    vec3 ambient = uAmbient * uSkyColor;
    
    // Итоговый цвет = цвет биома * освещение
    vec3 lighting = ambient + diffuse;
    vec3 result = Color * lighting;
    
    // Туман по высоте (опционально)
    float fogFactor = clamp((Height - 50.0) / 150.0, 0.0, 1.0);
    result = mix(result, uSkyColor, fogFactor * 0.3);
    
    FragColor = vec4(result, 1.0);
}
