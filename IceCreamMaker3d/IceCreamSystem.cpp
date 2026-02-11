// IceCreamSystem.cpp
#include "IceCreamSystem.h"
#include "Util.h"
#include <algorithm>

IceCreamSystem::IceCreamSystem(float iceSpeed, float contactGraceTime, float cupContactY, float pourStartY, float pourEndY)
    : iceT(0.0f),
    speed(iceSpeed),
    grace(0.0f),
    graceTime(contactGraceTime)
{
    float t = (cupContactY - pourStartY) / (pourEndY - pourStartY);
    tContactCached = clamp01(t);
}

void IceCreamSystem::reset()
{
    iceT = 0.0f;
    grace = 0.0f;
}

void IceCreamSystem::update(float dt, bool pourOrStop, bool contactNow)
{
    if (contactNow)
        grace = graceTime;
    else
        grace = std::max(0.0f, grace - dt);

    if (pourOrStop && grace > 0.0f)
        iceT = std::min(1.0f, iceT + dt * speed);
}

float IceCreamSystem::progress() const
{
    return iceT;
}

float IceCreamSystem::contactT() const
{
    return tContactCached;
}
