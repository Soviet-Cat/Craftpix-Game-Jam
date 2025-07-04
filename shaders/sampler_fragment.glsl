#version 300 es

precision mediump float;

in vec2 fragTexCoord;

uniform sampler2D tex;
uniform vec2 size;
uniform vec2 portion;
uniform bool flipX;
uniform bool flipY;

out vec4 color;

void main()
{
    float flippedX = mix(fragTexCoord.x, 1.0 - fragTexCoord.x, flipX);
    float flippedY = mix(fragTexCoord.y, 1.0 - fragTexCoord.y, flipY);

    vec2 finalTexCoords = vec2(
        portion.x + (size.x * flippedX),
        portion.y + (size.y * flippedY)
    );

    color = texture(tex, vec2(finalTexCoords.x, 1.0 - finalTexCoords.y));
}