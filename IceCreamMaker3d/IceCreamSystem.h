// IceCreamSystem.h
#pragma once

class IceCreamSystem
{
public:
    IceCreamSystem(float iceSpeed, float contactGraceTime, float cupContactY, float pourStartY, float pourEndY);

    void reset();

    // contactNow = "pour has reached cup" (or just past contact threshold)
    void update(float dt, bool pourOrStop, bool contactNow);

    float progress() const;  // iceT [0..1]
    float contactT() const;  // threshold tContact [0..1]

private:
    float iceT;
    float speed;

    float grace;
    float graceTime;

    float tContactCached;
};
