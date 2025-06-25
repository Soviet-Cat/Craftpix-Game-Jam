#version 300 es

precision mediump float;

in vec3 fragColor;
in float fragTime;

layout(location=0) out vec4 outputColor;

void main()
{
    outputColor = vec4(fragColor, sin(fragTime));
}