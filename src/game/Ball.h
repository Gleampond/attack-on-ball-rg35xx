// Simple bouncing ball that rolls left to right.
#pragma once

#include <SDL2/SDL.h>
#include <string>

class Ball {
public:
    Ball();
    ~Ball();

    bool LoadTexture(SDL_Renderer* renderer, const std::string& path);
    void UnloadTexture();

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
    float texture_scale_{0.25f};
    SDL_Texture* texture_{nullptr};
    int texture_width_{0};
    int texture_height_{0};

    void ClampAndBounce();
    void FreeTexture();
    void ResetToGround();
};
