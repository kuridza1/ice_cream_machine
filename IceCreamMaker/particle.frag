#version 330 core
in vec2 TexCoord;
out vec4 FragColor;

uniform vec3 uColor;

void main() {
    // Create circular particles
    vec2 center = vec2(0.5, 0.5);
    float dist = distance(TexCoord, center);
    if (dist > 0.5) {
        discard;
    }
    FragColor = vec4(uColor, 1.0);
}