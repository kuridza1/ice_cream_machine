#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

uniform vec2 uPosition;
uniform float uSize;
uniform vec2 uScreenSize;  // Add this uniform

out vec2 TexCoord;

void main() {
    vec2 scaledPos = aPos * uSize;
    
    // Apply aspect ratio correction
    float aspect = uScreenSize.x / uScreenSize.y;
    scaledPos.x *= aspect;
    
    vec2 finalPos = scaledPos + uPosition;
    gl_Position = vec4(finalPos, 0.0, 1.0);
    TexCoord = aTexCoord;
}