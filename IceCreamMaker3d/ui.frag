#version 330 core
out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uTex;
uniform float uAlpha; // 0..1

void main()
{
    vec4 t = texture(uTex, vUV);
    FragColor = vec4(t.rgb, t.a * uAlpha);
}
