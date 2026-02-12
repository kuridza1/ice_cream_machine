// PowerSystem.cpp
#include "PowerSystem.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

static inline float signf(float x) { return (x > 0.0f) - (x < 0.0f); }


void PowerSystem::init(const glm::vec3& pivotLocal,
    const glm::vec3& axisLocal,
    float offAngleDeg,
    float onAngleDeg,
    float speedDegPerSec)
{
    m_pivotLocal = pivotLocal;

    if (glm::length(axisLocal) > 0.000001f) m_axisLocal = glm::normalize(axisLocal);
    else m_axisLocal = glm::vec3(1, 0, 0);

    m_offAngleDeg = offAngleDeg;
    m_onAngleDeg = onAngleDeg;
    m_speedDegPerS = speedDegPerSec;

    m_on = false;
    m_currentAngleDeg = m_offAngleDeg;
    m_targetAngleDeg = m_offAngleDeg;

    m_prevKeyDown = false;
}

void PowerSystem::setOn(bool on)
{
    m_on = on;
    m_targetAngleDeg = m_on ? m_onAngleDeg : m_offAngleDeg;
}

void PowerSystem::toggle()
{
    setOn(!m_on);
}

bool PowerSystem::isOn() const
{
    return m_on;
}


void PowerSystem::update(float dt)
{
    float diff = m_targetAngleDeg - m_currentAngleDeg;
    float step = m_speedDegPerS * dt;

    if (std::abs(diff) <= step)
        m_currentAngleDeg = m_targetAngleDeg;
    else
        m_currentAngleDeg += signf(diff) * step;
}

glm::mat4 PowerSystem::applyTo(const glm::mat4& buttonModel) const
{
    glm::mat4 M = buttonModel;

    // Rotate around LOCAL pivot (after buttonModel transforms).
    M = glm::translate(M, m_pivotLocal);
    M = glm::rotate(M, glm::radians(m_currentAngleDeg), m_axisLocal);
    M = glm::translate(M, -m_pivotLocal);

    return M;
}

float PowerSystem::angleDeg() const
{
    return m_currentAngleDeg;
}
