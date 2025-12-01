// Player entity representation and controls.
#pragma once

#include <SDL2/SDL.h>

class Player {
public:
    Player();

    void HandleInput(const Uint8* keyboard_state);
    void Update(float delta_seconds);
    void Render(SDL_Renderer* renderer) const;

private:
    SDL_Rect rect_{};
    float speed_pixels_per_second_{220.0f};
    float velocity_x_{0.0f};

    void ClampToScreen();
};
