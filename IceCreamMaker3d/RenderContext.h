// RenderContext.h
#pragma once
#include <glm/glm.hpp>

struct RenderContext
{
    glm::mat4 P;
    glm::mat4 V;

    glm::vec3 lightPos;
    glm::vec3 viewPos;
    glm::vec3 lightColor;
};
