#pragma once

#include "game/State.h"

class BootState : public State {
public:
    void Enter(Game& game) override;
    void Exit(Game& game) override;
    void HandleEvent(Game& game, const SDL_Event& event) override;
    void Update(Game& game, float delta_seconds) override;
    void Render(Game& game) override;
};
