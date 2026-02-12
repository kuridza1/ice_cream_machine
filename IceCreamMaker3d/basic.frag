#version 330 core
out vec4 FragColor;

in vec3 chNormal;
in vec3 chFragPos;
in vec2 chUV;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

// NEW
uniform int uUseProgressMask;   // 0/1
uniform float uProgress;        // 0..1
uniform sampler2D uDiffMap1;

void main()
{
    if (uUseProgressMask == 1)
    {
        // Pretpostavka: chUV.x je 0..1 duž krive
        if (chUV.x > uProgress)
            discard;
    }

    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * uLightColor;

    vec3 norm = normalize(chNormal);
    vec3 lightDir = normalize(uLightPos - chFragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor;

    float specularStrength = 0.5;
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = specularStrength * spec * uLightColor;
    vec4 tex = texture(uDiffMap1, chUV);

    vec3 lit = (ambient + diffuse + specular) * tex.rgb;
    FragColor = vec4(lit, tex.a); 
}
