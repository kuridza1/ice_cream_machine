#include <glm/ext/vector_float3.hpp>
#include <glm/ext/matrix_float4x4.hpp>
struct RenderContext {
    glm::mat4 P;
    glm::mat4 V;
    glm::vec3 lightPos;
    glm::vec3 viewPos;
    glm::vec3 lightColor;
};
