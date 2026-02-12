#pragma once
#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <array>

enum class IceFlavor : int { One = 0, Two = 1, Mix = 2 };
enum class ButtonId  : int { One = 0, Two = 1, Mix = 2 };

class ButtonSystem
{
public:
    ButtonSystem() = default;

    void init(float pressDepth = 0.06f, float pressSpeed = 18.0f, float releaseSpeed = 12.0f,
              glm::vec3 pressAxisLocal = glm::vec3(0, 0, -1));

    void update(float dt);
    void reset();
    void press(ButtonId id);

    IceFlavor selectedFlavor() const { return m_selected; }
    glm::mat4 applyTo(const glm::mat4& base, ButtonId id) const;

private:
    struct BtnState { float t = 0.0f; bool pressing = false; };
    std::array<BtnState, 3> m_btn{};
    IceFlavor m_selected = IceFlavor::One;

    float m_pressDepth = 0.01f;
    float m_pressSpeed = 18.0f;
    float m_releaseSpeed = 12.0f;
    glm::vec3 m_axis = glm::vec3(0, 0, -1);
};
