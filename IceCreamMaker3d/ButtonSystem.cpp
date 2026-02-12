#include "ButtonSystem.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

void ButtonSystem::init(float pressDepth, float pressSpeed, float releaseSpeed, glm::vec3 pressAxisLocal)
{
    m_pressDepth = pressDepth;
    m_pressSpeed = pressSpeed;
    m_releaseSpeed = releaseSpeed;

    if (glm::length(pressAxisLocal) > 1e-6f) m_axis = glm::normalize(pressAxisLocal);
    else m_axis = glm::vec3(0, 0, -1);

    for (auto& b : m_btn) { b.t = 0.0f; b.pressing = false; }
    m_selected = IceFlavor::One;
}

void ButtonSystem::press(ButtonId id)
{
    if (id == ButtonId::One) m_selected = IceFlavor::One;
    if (id == ButtonId::Two) m_selected = IceFlavor::Two;
    if (id == ButtonId::Mix) m_selected = IceFlavor::Mix;
}

void ButtonSystem::update(float dt)
{
    for (int i = 0; i < 3; i++)
    {
        bool shouldBeDown = ((int)m_selected == i);

        if (shouldBeDown)
        {
            m_btn[i].t += m_pressSpeed * dt;
            if (m_btn[i].t > 1.0f) m_btn[i].t = 1.0f;
        }
        else
        {
            m_btn[i].t -= m_releaseSpeed * dt;
            if (m_btn[i].t < 0.0f) m_btn[i].t = 0.0f;
        }
    }
}

void ButtonSystem::reset()
{
    m_selected = IceFlavor::One;
    for (auto& b : m_btn) b.t = 0.0f;
}

glm::mat4 ButtonSystem::applyTo(const glm::mat4& base, ButtonId id) const
{
    int i = (int)id;
    float depth = m_pressDepth * m_btn[i].t;

    glm::mat4 M = base;
    M = glm::translate(M, m_axis * depth);
    return M;
}
