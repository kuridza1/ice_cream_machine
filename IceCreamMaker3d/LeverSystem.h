// LeverSystem.h
#pragma once
#include <glm/glm.hpp>

class LeverSystem
{
public:
    // maxAngleRad: npr 60° (glm::radians(60.0f))
    // pivotLocal: pivot u prostoru "base" transformacije (moraš po potrebi dotegnuti)
    // axisLocal: osa rotacije (u prostoru "base"), npr (1,0,0) ili (0,0,1)
    // speed: brzina prilaska target-u (1/s), npr 8.0f
    LeverSystem(float maxAngleRad,
        glm::vec3 pivotLocal,
        glm::vec3 axisLocal,
        float speed);

    void toggle();     // SPACE: spusti/podigni (target)
    void resetUp();    // reset na gore

    void update(float dt);

    // true tek kad je gotovo spušten (za "tek tada krene sipanje")
    bool isFullyDown(float eps = 0.001f) const;

    // transform za crtanje poluge
    glm::mat4 apply(const glm::mat4& base) const;

private:
    float maxAngle;
    glm::vec3 pivot;
    glm::vec3 axis;
    float animSpeed;

    bool targetDown; // gde poluga "želi" da ode
    float t;         // 0..1 (0=up, 1=down)
};
