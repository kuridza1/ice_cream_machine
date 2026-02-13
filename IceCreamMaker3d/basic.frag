#version 330 core
out vec4 FragColor;

in vec3 chNormal;
in vec3 chFragPos;
in vec2 chUV;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

uniform int uUseProgressMask;   
uniform float uProgress;       

uniform sampler2D uDiffMap1;    
uniform sampler2D uDiffMap2;    
uniform sampler2D uDiffMapMix;  
uniform int uFlavor;           

uniform int uUseEmission;       
uniform vec3 uEmissionColor;   
uniform float uEmissionStrength;

uniform int uUseBtnLight;

uniform vec3 uBtnLightPos;
uniform vec3 uBtnLightColor;
uniform float uBtnLightIntensity;


void main()
{
    if (uUseProgressMask == 1)
    {
        if (chUV.x > uProgress)
            discard;
    }

    vec4 tex;
    if (uFlavor == 1)      tex = texture(uDiffMap2, chUV);
    else if (uFlavor == 2) tex = texture(uDiffMapMix, chUV);
    else                   tex = texture(uDiffMap1, chUV);

    float ambientStrength = 0.08;
    vec3 ambient = ambientStrength * uLightColor;

    vec3 norm = normalize(chNormal);
    vec3 L = uLightPos - chFragPos;
    float dist = length(L);
    vec3 lightDir = normalize(L);

    float att = 1.0 / (1.0 + 0.5 * dist + 2.0 * dist * dist);

    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLightColor * att;

    float specularStrength = 0.3;
    vec3 viewDir = normalize(uViewPos - chFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64.0);
    vec3 specular = 0.7 * spec * uLightColor * att;
    vec3 lit = (ambient + diffuse + specular) * tex.rgb;

    if (uUseBtnLight == 1)
    {
        vec3 L = normalize(uBtnLightPos - chFragPos);
        float dist = length(uBtnLightPos - chFragPos);

        float att = 1.0 / (1.0 + 2.0 * dist + 6.0 * dist * dist);

        float diff2 = max(dot(norm, L), 0.0);
        vec3 diffuse2 = diff2 * uBtnLightColor * uBtnLightIntensity * att;

        vec3 reflectDir2 = reflect(-L, norm);
        float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);
        vec3 specular2 = spec2 * uBtnLightColor * 0.3 * uBtnLightIntensity * att;

        lit += (diffuse2 + specular2) * tex.rgb;
    }

    if (uUseEmission == 1)
        lit += uEmissionColor * uEmissionStrength;

    FragColor = vec4(lit, tex.a);

}
