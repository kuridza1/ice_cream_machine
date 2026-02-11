// CupController.h
#pragma once
#include <glm/glm.hpp>

class CupController
{
public:
    explicit CupController(float spinSpeedRad);

    void update(float dt);
    glm::mat4 apply(const glm::mat4& base) const;

private:
    float spinSpeed;
    float angle;
};
