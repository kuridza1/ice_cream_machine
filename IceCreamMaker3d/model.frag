#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColor;

uniform sampler2D uDiffMap1;

void main()
{
    vec3 albedo = texture(uDiffMap1, TexCoord).rgb;

    vec3 N = normalize(Normal);
    vec3 L = normalize(lightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);

    vec3 ambient = 0.15 * albedo;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * albedo * lightColor;

    vec3 R = reflect(-L, N);
    float spec = pow(max(dot(V, R), 0.0), 32.0);
    vec3 specular = 0.35 * spec * lightColor;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
