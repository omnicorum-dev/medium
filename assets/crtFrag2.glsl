#version 330 core

uniform vec2 u_resolution;

// Parameters
uniform float GLOW_FALLOFF = 0.05;
uniform float TAPS = 2.0;

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;

float kernel(float i)
{
    return exp(-GLOW_FALLOFF * i * i);
}

void main()
{
    vec4 col = vec4(0.0);
    float dx = 4.0 / u_resolution.x;

    float k_total = 0.0;
    for (float i = -TAPS; i <= TAPS; i++)
    {
        float k   = kernel(i);
        k_total  += k;
        col      += k * texture(screenTexture, TexCoord + vec2(i * dx, 0.0)).rgba;
    }

    FragColor = vec4(col / k_total);
}