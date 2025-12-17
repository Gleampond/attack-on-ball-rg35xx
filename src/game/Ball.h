// Simple bouncing ball that rolls left to right.
#pragma once

#include <SDL2/SDL.h>

class Ball {
public:
    Ball();

    void Update(float delta_seconds);
    void Render(SDL_Renderer* renderer) const;

private:
    float x_{0.0f};
    float y_{0.0f};
    float radius_{10.0f};
    float velocity_x_{140.0f};
    float velocity_y_{0.0f};
    float gravity_{900.0f};
    float bounce_speed_{520.0f};

    void ClampAndBounce();
};
