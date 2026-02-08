#pragma once

#include <SDL2/SDL.h>
#include <string>

#include "game/ScoreStorage.h"
#include "game/State.h"

class ResultState : public State {
public:
    ResultState(int best_score, int your_score, int land_index);

    void Enter(Game& game) override;
    void Exit(Game& game) override;
    void HandleEvent(Game& game, const SDL_Event& event) override;
    void Update(Game& game, float delta_seconds) override;
    void Render(Game& game) override;

private:
    struct Button {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        bool pressed = false;
        std::string key_base;
    };

    int best_score_ = 0;
    int your_score_ = 0;
    int land_index_ = 0;
    float elapsed_ = 0.0f;
    float your_flash_timer_ = 0.0f;
    SDL_Color your_flash_color_{0, 0, 0, 255};

    Button gamecenter_{};
    Button share_{};
    Button play_{};

    ScoreStorage storage_;

    bool IsInside(const Button& button, float x, float y) const;
};
