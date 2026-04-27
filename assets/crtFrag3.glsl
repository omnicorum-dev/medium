#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 u_resolution;
uniform int u_frame;

// --- parameters ---
uniform float LPY    = 0.25;   // luma low-pass falloff
uniform float LPC    = 0.5;  // chroma low-pass falloff
uniform float c_gain = 1.5;   // chroma gain
uniform float u_comb = 0.9;   // comb filter strength (0=off, 1=full)
uniform float phase_choose   = 2.0; // vertical phase period (lines)
uniform float h_phase_choose = 3.0; // horizontal phase period (samples)

#define TAU 6.2831853

vec3 rgb2yiq(vec3 rgb) {
    float y = dot(rgb, vec3(0.299,    0.587,    0.114));
    float i = dot(rgb, vec3(0.595716,-0.274453,-0.321263));
    float q = dot(rgb, vec3(0.211456,-0.522591, 0.311135));
    return vec3(y, i, q);
}

vec3 yiq2rgb(vec3 yiq) {
    float r = yiq.x + 0.9563 * yiq.y + 0.6210 * yiq.z;
    float g = yiq.x - 0.2721 * yiq.y - 0.6474 * yiq.z;
    float b = yiq.x - 1.1070 * yiq.y + 1.7046 * yiq.z;
    return vec3(r, g, b);
}

void main()
{
    vec2 invdims = 1.0 / u_resolution;

    float timer = mod(float(u_frame + 1), 2.0) * TAU * 0.5;

    float line_phase = mod(floor(TexCoord.y * u_resolution.y), phase_choose)
    * TAU / phase_choose;

    vec3  final = vec3(0.0);
    float sumY  = 0.0;
    float sumC  = 0.0;

    vec2 dx = vec2(invdims.x, 0.0);
    vec2 dy = vec2(0.0, invdims.y * 0.25);

    for (int i = -1; i < 2; i++)
    {
        float n  = float(i);
        float wY = exp(-LPY * n * n);
        float wC = exp(-LPC * n * n);

        float phase = (TexCoord.x + n * invdims.x) * u_resolution.x * 170.666
        * TAU / h_phase_choose;
        phase += timer;
        phase += line_phase;

        float cs = cos(phase);
        float sn = sin(phase);

        vec3 burst1 = vec3( 1.0,  cs,  sn);
        vec3 burst2 = vec3( 1.0, -cs, -sn);

        vec3 res1 = rgb2yiq(texture(screenTexture, TexCoord + n * dx     ).rgb);
        vec3 res2 = rgb2yiq(texture(screenTexture, TexCoord + n * dx - dy).rgb);

        res1 *= burst1;
        res2 *= burst2;

        float comp1 = dot(vec3(1.0), res1);
        float comp2 = dot(vec3(1.0), res2);

        float luma = (comp1 + comp2) * 0.5;
        final.r   += luma * wY;

        vec2 chroma = (comp1 - luma * u_comb) * wC * burst1.gb * c_gain;
        final.g += chroma.x;
        final.b += chroma.y + chroma.x * 0.15;

        sumY += wY;
        sumC += wC;
    }

    final.r  /= sumY;
    final.gb /= sumC;

    FragColor = vec4(yiq2rgb(final), 1.0);
}