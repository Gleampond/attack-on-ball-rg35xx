// Core game loop management.
#pragma once

#include <SDL2/SDL.h>

#include "Ball.h"
#include "Player.h"

class Game {
public:
    Game();
    ~Game();

    bool Init();
    void Run();

private:
    void ProcessInput();
    void Update(float delta_seconds);
    void Render();
    void Shutdown();

    SDL_Window* window_{nullptr};
    SDL_Renderer* renderer_{nullptr};
    bool is_running_{true};
    Player player_{};
    Ball ball_{};
    Uint32 previous_ticks_{0};
    float elapsed_time_{0.0f};
};
