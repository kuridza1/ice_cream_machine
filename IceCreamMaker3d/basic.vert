#version 330 core
layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

out vec3 chFragPos;
out vec3 chNormal;
out vec2 chUV;

uniform mat4 uM;
uniform mat4 uV;
uniform mat4 uP;

void main()
{
    chUV = inUV;

    vec4 worldPos = uM * vec4(inPos, 1.0);
    chFragPos = worldPos.xyz;

    // normal matrix (handles scaling correctly)
    mat3 normalMat = transpose(inverse(mat3(uM)));
    chNormal = normalize(normalMat * inNormal);

    gl_Position = uP * uV * worldPos;
}
