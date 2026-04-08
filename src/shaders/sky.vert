#version 330 core
layout(location = 0) in vec3 aPos;
out vec3 vDir;
uniform mat4 uView;
uniform mat4 uProj;
void main() {
    // Направление луча камеры через эту точку сферы
    vDir = normalize(aPos);
    
    // Рисуем далеко, с глубиной на фоне
    gl_Position = uProj * mat4(mat3(uView)) * vec4(aPos * 500.0, 1.0);
    gl_Position.z = gl_Position.w * 0.999;
}
