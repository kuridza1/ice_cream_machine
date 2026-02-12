#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

void main()
{
    vec4 worldPos = M * vec4(aPos, 1.0);
    FragPos = worldPos.xyz;
    Normal = mat3(transpose(inverse(M))) * aNormal;
    TexCoord = aTexCoord;

    gl_Position = P * V * worldPos;
}
