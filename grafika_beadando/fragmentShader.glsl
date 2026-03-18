#version 330 core
out vec4 FragColor;

uniform vec2 u_resolution;
uniform vec2 u_circleCenter;
uniform vec3 u_innerColor;
uniform vec3 u_outerColor;
uniform float u_lineY;

void main() {
    vec2 pixelPos = gl_FragCoord.xy;
    vec3 color = vec3(1.0, 1.0, 0.0); // 1. Kötelezõ sárga háttér

    float radius = 50.0;
    float dist = distance(pixelPos, u_circleCenter);

    // 2. Kötelezõ kör rajzolása interpolációval
    if (dist < radius) {
        float t = dist / radius;
        color = mix(u_innerColor, u_outerColor, t);
    }

    // 4. Kötelezõ kék szakasz: 1/3 szélesség (200-400), 3 pixel vastagság (abs <= 1.5)
    if (pixelPos.x >= 200.0 && pixelPos.x <= 400.0 && abs(pixelPos.y - u_lineY) <= 1.5) {
        color = vec3(0.0, 0.0, 1.0);
    }

    FragColor = vec4(color, 1.0);
}