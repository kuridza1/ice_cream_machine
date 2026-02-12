#version 330 core
out vec4 FragColor;

in vec3 chNormal;
in vec3 chFragPos;
in vec2 chUV;

uniform vec3 uLightPos;
uniform vec3 uViewPos;
uniform vec3 uLightColor;

// progress mask (ice cream growth)
uniform int uUseProgressMask;   // 0/1
uniform float uProgress;        // 0..1

// textures
uniform sampler2D uDiffMap1;    // flavor 1
uniform sampler2D uDiffMap2;    // flavor 2
uniform sampler2D uDiffMapMix;  // mix
uniform int uFlavor;            // 0=1, 1=2, 2=mix

// emission (LED / glowing button part)
uniform int uUseEmission;       // 0/1
uniform vec3 uEmissionColor;    // npr. (0,1,0)
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

    // pick texture by flavor
    vec4 tex;
    if (uFlavor == 1)      tex = texture(uDiffMap2, chUV);
    else if (uFlavor == 2) tex = texture(uDiffMapMix, chUV);
    else                   tex = texture(uDiffMap1, chUV);

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
    vec3 lit = (ambient + diffuse + specular) * tex.rgb;

    // --- BUTTON POINT LIGHT ---
    if (uUseBtnLight == 1)
    {
        vec3 L = normalize(uBtnLightPos - chFragPos);
        float dist = length(uBtnLightPos - chFragPos);

        // attenuation
        float att = 1.0 / (1.0 + 2.0 * dist + 6.0 * dist * dist);

        float diff2 = max(dot(norm, L), 0.0);
        vec3 diffuse2 = diff2 * uBtnLightColor * uBtnLightIntensity * att;

        vec3 reflectDir2 = reflect(-L, norm);
        float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 32.0);
        vec3 specular2 = spec2 * uBtnLightColor * 0.3 * uBtnLightIntensity * att;

        lit += (diffuse2 + specular2) * tex.rgb;
    }

    // emission (self glow)
    if (uUseEmission == 1)
        lit += uEmissionColor * uEmissionStrength;

    FragColor = vec4(lit, tex.a);

}
