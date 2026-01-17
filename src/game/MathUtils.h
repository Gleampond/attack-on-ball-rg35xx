#pragma once

#include <algorithm>
#include <cmath>

inline float ClampFloat(float value, float min_value, float max_value) {
    return std::max(min_value, std::min(value, max_value));
}

inline float Lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline float EaseOutCubic(float t) {
    const float p = t - 1.0f;
    return p * p * p + 1.0f;
}

inline float EaseOutBounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    }
    if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    }
    if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    }
    t -= 2.625f / 2.75f;
    return 7.5625f * t * t + 0.984375f;
}
