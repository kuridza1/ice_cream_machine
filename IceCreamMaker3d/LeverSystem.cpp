// LeverSystem.cpp
#include "LeverSystem.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

LeverSystem::LeverSystem(float maxAngleRad,
    glm::vec3 pivotLocal,
    glm::vec3 axisLocal,
    float speed)
    : maxAngle(maxAngleRad),
    pivot(pivotLocal),
    axis(glm::normalize(axisLocal)),
    animSpeed(speed),
    targetDown(false),
    t(0.0f)
{
}

void LeverSystem::toggle()
{
    targetDown = !targetDown;
}

void LeverSystem::resetUp()
{
    targetDown = false;
    t = 0.0f;
}

void LeverSystem::update(float dt)
{
    // Smooth-ish linear approach (kritično damping-ovano nije potrebno ovde)
    float targetT = targetDown ? 1.0f : 0.0f;

    if (t < targetT)
        t = std::min(targetT, t + animSpeed * dt);
    else if (t > targetT)
        t = std::max(targetT, t - animSpeed * dt);
}

bool LeverSystem::isFullyDown(float eps) const
{
    return t >= 1.0f - eps;
}

glm::mat4 LeverSystem::apply(const glm::mat4& base) const
{
    // rotacija oko pivot tačke u prostoru "base"
    glm::mat4 m = base;
    m = glm::translate(m, pivot);
    m = glm::rotate(m, -maxAngle * t, axis);   // '-' da ide "ka dole"; okreni znak ako ide naopako
    m = glm::translate(m, -pivot);
    return m;
}
