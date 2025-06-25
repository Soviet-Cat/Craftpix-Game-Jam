#version 300 es

layout(location=0) in vec2 position;
layout(location=1) in vec3 color;

uniform float time;

out vec3 fragColor;
out float fragTime;

void main()
{
    fragTime = time;
    gl_Position = vec4(position.x, position.y + sin(time), 1.0f, 1.0f);
    fragColor = color;
}