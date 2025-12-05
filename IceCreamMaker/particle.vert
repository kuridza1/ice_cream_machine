#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 uPosition;
uniform float uSize;

out vec2 TexCoord;

void main() {
    vec2 scaledPos = aPos * uSize;
    vec2 finalPos = scaledPos + uPosition;
    gl_Position = vec4(finalPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}