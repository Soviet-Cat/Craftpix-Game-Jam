#version 300 es

precision mediump float;

in vec2 fragTexCoord;

uniform sampler2D tex;
uniform vec2 offset;
uniform float layer;
uniform float maxLayer;

out vec4 color;

void main()
{
    float layerHeight = 1.0 / maxLayer;
    float offsetScale = 0.5 / maxLayer * layer;
    vec2 uv = fragTexCoord + vec2(offset.x * offsetScale, offset.y * offsetScale);

    uv.y = fract(uv.y);
    uv.y = uv.y * layerHeight + layer * layerHeight;

    float loopedY = mod(fract(uv.y), layerHeight);
    if (loopedY < 0.0) loopedY += layerHeight;

    uv.y = loopedY + layer * layerHeight;

    color = texture(tex, uv);
}