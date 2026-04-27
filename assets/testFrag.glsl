#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec2  u_scale;
uniform vec2  u_offset;
uniform vec2  u_resolution;
uniform vec2  u_mouse;
uniform float u_time;
uniform float u_deltaTime;

uniform sampler2D screenTexture;

void main()
{
    float waveSpacing = u_resolution.y * 2;
    float waveSpeed = 200.0;
    float waveAmount = 0.0015;

    vec2 uv = vec2(TexCoord.x, TexCoord.y);
    float shift = sin(uv.y * waveSpacing + u_time * waveSpeed) * waveAmount;
    uv.x = fract(uv.x + shift);
    FragColor = texture(screenTexture, uv);
}