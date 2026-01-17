#pragma once

#include <SDL2/SDL.h>

class Game;

class State {
public:
    virtual ~State() = default;
    virtual void Enter(Game& game) = 0;
    virtual void Exit(Game& game) = 0;
    virtual void HandleEvent(Game& game, const SDL_Event& event) = 0;
    virtual void Update(Game& game, float delta_seconds) = 0;
    virtual void Render(Game& game) = 0;
};
