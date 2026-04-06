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

void main() {
    FragPos = aPos;
    Normal = aNormal;
    Color = aColor;
    Height = aPos.y;
    
    gl_Position = uProj * uView * vec4(aPos, 1.0);
}
