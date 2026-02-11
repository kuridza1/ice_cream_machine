// CupController.cpp
#include "CupController.h"
#include <glm/gtc/matrix_transform.hpp>

CupController::CupController(float spinSpeedRad)
    : spinSpeed(spinSpeedRad), angle(0.0f)
{
}

void CupController::update(float dt)
{
    angle += spinSpeed * dt;
}

glm::mat4 CupController::apply(const glm::mat4& base) const
{
    glm::mat4 m = base;
    m = glm::rotate(m, -angle, glm::vec3(0.0f, 1.0f, 0.0f));
    return m;
}
