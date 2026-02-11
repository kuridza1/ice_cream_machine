// Util.h
#pragma once
#include <algorithm>

static inline float clamp01(float x)
{
    return std::max(0.0f, std::min(1.0f, x));
}
