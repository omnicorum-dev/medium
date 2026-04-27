#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 u_resolution;
uniform float u_time;

// ---------- parameters ----------
uniform float SCANLINE_STRENGTH = 0.9;
uniform float SCANLINE_DARKNESS = 0.45;
uniform float SCANLINE_HEIGHT = 1.0;
uniform float CURVATURE = 0.03;
uniform float VIGNETTE_STRENGTH = 0.35;
uniform float CHROMA_SHIFT = 0.0015;

// ---------- helpers ----------
vec2 curve_uv(vec2 uv)
{
    uv = uv * 2.0 - 1.0;
    uv.x *= 1.0 + (uv.y * uv.y) * CURVATURE;
    uv.y *= 1.0 + (uv.x * uv.x) * CURVATURE;
    return uv * 0.5 + 0.5;
}

void main()
{
    vec4 src = texture(screenTexture, TexCoord);
    vec2 uv = curve_uv(TexCoord);

    // outside screen → black
    if (uv.x < 0.0 || uv.x > 1.0 ||
    uv.y < 0.0 || uv.y > 1.0)
    {
        FragColor = vec4(0.0);
        return;
    }

    // chromatic aberration (cheap CRT feel)
    float r = texture(screenTexture, uv + vec2(CHROMA_SHIFT, 0.0)).r;
    float g = texture(screenTexture, uv).g;
    float b = texture(screenTexture, uv - vec2(CHROMA_SHIFT, 0.0)).b;

    vec3 col = vec3(r, g, b);

    // scanlines
    float scan = sin(uv.y * u_resolution.y * SCANLINE_HEIGHT * 3.14159);
    float scanline = mix(1.0, SCANLINE_DARKNESS, scan * SCANLINE_STRENGTH);

    col *= scanline;

    // vignette
    vec2 center = uv - 0.5;
    float vignette = smoothstep(0.8, 0.2, dot(center, center));
    col *= mix(1.0, vignette, VIGNETTE_STRENGTH);

    FragColor = vec4(col, src.a);
}