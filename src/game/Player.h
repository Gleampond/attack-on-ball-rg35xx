// Player entity representation and controls.
#pragma once

#include <SDL2/SDL.h>
#include <string>

class Player {
public:
    Player();
    ~Player();

    bool LoadTexture(SDL_Renderer* renderer, const std::string& path);
    void UnloadTexture();

    void HandleInput(const Uint8* keyboard_state);
    void Update(float delta_seconds);
    void Render(SDL_Renderer* renderer) const;

private:
    SDL_Rect rect_{};
    float speed_pixels_per_second_{220.0f};
    float velocity_x_{0.0f};
    SDL_Texture* texture_{nullptr};

    void ClampToScreen();
    void FreeTexture();
};
