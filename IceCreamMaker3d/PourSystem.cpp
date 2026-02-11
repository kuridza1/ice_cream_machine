// PourSystem.cpp
#include "PourSystem.h"
#include "shader.hpp"
#include "model.hpp"
#include "Util.h"
#include <glm/gtc/matrix_transform.hpp>

PourSystem::PourSystem(float speed, float overlap, float startY, float endY)
    : pourSpeed(speed),
    overlapK(overlap),
    startOffset(0.0f, startY, 0.0f),
    endOffset(0.0f, endY, 0.0f),
    st(State::Idle),
    t1(0.0f),
    t2(0.0f),
    p1Active(true),
    p2Active(false)
{
}

void PourSystem::toggle()
{
    if (st == State::Idle)
    {
        st = State::Pouring;
        return;
    }

    if (st == State::Pouring)
    {
        st = State::Stopping; // no new spawns, let active finish
        return;
    }

    if (st == State::Stopping)
    {
        st = State::Pouring; // cancel graceful stop, continue spawning
        return;
    }
}

void PourSystem::reset()
{
    st = State::Idle;

    t1 = 0.0f;
    t2 = 0.0f;
    p1Active = true;
    p2Active = false;
}

PourSystem::State PourSystem::state() const
{
    return st;
}

bool PourSystem::isActiveOrStopping() const
{
    return st == State::Pouring || st == State::Stopping;
}

void PourSystem::update(float dt)
{
    if (!isActiveOrStopping())
        return;

    float step = dt * pourSpeed;

    if (p1Active) t1 += step;
    if (p2Active) t2 += step;

    // Start next ONLY while Pouring (not during Stopping)
    if (st == State::Pouring)
    {
        if (p1Active && !p2Active && t1 >= 1.0f - overlapK)
        {
            p2Active = true;
            t2 = 0.0f;
        }

        if (p2Active && !p1Active && t2 >= 1.0f - overlapK)
        {
            p1Active = true;
            t1 = 0.0f;
        }
    }

    // Finish instances (always)
    if (p1Active && t1 >= 1.0f)
    {
        p1Active = false;
        t1 = 0.0f;
    }

    if (p2Active && t2 >= 1.0f)
    {
        p2Active = false;
        t2 = 0.0f;
    }

    // If stopping and both done -> fully stopped, reset to ready-for-next-start
    if (st == State::Stopping && !p1Active && !p2Active)
    {
        st = State::Idle;

        t1 = 0.0f;
        t2 = 0.0f;
        p1Active = true;
        p2Active = false;
    }
}

bool PourSystem::isContactPast(float tContact) const
{
    bool c = false;
    if (p1Active && clamp01(t1) > tContact) c = true;
    if (p2Active && clamp01(t2) > tContact) c = true;
    return c;
}

void PourSystem::draw(const glm::mat4& base, Shader& shader, Model& pourModel) const
{
    auto drawAt = [&](float t)
        {
            glm::vec3 off = glm::mix(startOffset, endOffset, clamp01(t));
            glm::mat4 m = base;
            m = glm::translate(m, off);
            shader.setMat4("uM", m);
            pourModel.Draw(shader);
        };

    if (p1Active) drawAt(t1);
    if (p2Active) drawAt(t2);
}
// PourSystem.cpp (dodaj implementacije)
void PourSystem::start()
{
    if (st == State::Idle || st == State::Stopping)
        st = State::Pouring;
}

void PourSystem::requestStop()
{
    if (st == State::Pouring)
        st = State::Stopping;
}

bool PourSystem::isPouring() const { return st == State::Pouring; }
bool PourSystem::isStopping() const { return st == State::Stopping; }
bool PourSystem::isIdle() const { return st == State::Idle; }
