#include "BootState.h"

#include "game/Game.h"
#include "game/states/PreloadState.h"

void BootState::Enter(Game& game) {
    game.ChangeState(std::make_unique<PreloadState>());
}

void BootState::Exit(Game& game) {
    (void)game;
}

void BootState::HandleEvent(Game& game, const SDL_Event& event) {
    (void)game;
    (void)event;
}

void BootState::Update(Game& game, float delta_seconds) {
    (void)game;
    (void)delta_seconds;
}

void BootState::Render(Game& game) {
    (void)game;
}
