// PowerSystem.h
#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>

struct GLFWwindow;

class PowerSystem
{
public:
    PowerSystem() = default;

    PowerSystem(const glm::vec3& pivotLocal,
        const glm::vec3& axisLocal = glm::vec3(1, 0, 0),
        float offAngleDeg = 0.0f,
        float onAngleDeg = -30.0f,
        float speedDegPerSec = 220.0f)
    {
        init(pivotLocal, axisLocal, offAngleDeg, onAngleDeg, speedDegPerSec);
    }

    void init(const glm::vec3& pivotLocal,
        const glm::vec3& axisLocal = glm::vec3(1, 0, 0),
        float offAngleDeg = 0.0f,
        float onAngleDeg = -30.0f,
        float speedDegPerSec = 220.0f);

    void update(float dt);

    void setOn(bool on);
    void toggle();
    bool isOn() const;

    // Prosledi "buttonModel" = matrica koja pozicionira res.power u svetu (translate/scale/rot).
    // Pivot i osa su lokalni, pa se primenjuju stabilno.
    glm::mat4 applyTo(const glm::mat4& buttonModel) const;

    float angleDeg() const;

private:
    glm::vec3 m_pivotLocal{ 0.0f };
    glm::vec3 m_axisLocal{ 1.0f, 0.0f, 0.0f };

    bool  m_on = false;

    float m_offAngleDeg = 0.0f;
    float m_onAngleDeg = -30.0f;
    float m_speedDegPerS = 220.0f;

    float m_currentAngleDeg = 0.0f;
    float m_targetAngleDeg = 0.0f;

    bool m_prevKeyDown = false;
};
