// PourSystem.h
#pragma once
#include <glm/glm.hpp>

class Shader;
class Model;

class PourSystem
{
public:
    enum class State
    {
        Idle,
        Pouring,
        Stopping
    };

    PourSystem(float speed, float overlap, float startY, float endY);

    // SPACE behavior: Idle -> Pouring, Pouring -> Stopping, Stopping -> Pouring
    void toggle();

    void reset();
    void update(float dt);

    State state() const;
    bool isActiveOrStopping() const;

    // contactNow helper
    bool isContactPast(float tContact) const;

    void draw(const glm::mat4& base, Shader& shader, Model& pourModel) const;
    // PourSystem.h (dodaj u public)
    void start();        // Idle/Stopping -> Pouring
    void requestStop();  // Pouring -> Stopping
    bool isPouring() const;
    bool isStopping() const;
    bool isIdle() const;

private:
    float pourSpeed;
    float overlapK;
    glm::vec3 startOffset;
    glm::vec3 endOffset;

    State st;

    float t1, t2;
    bool p1Active, p2Active;
};
