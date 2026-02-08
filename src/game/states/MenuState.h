#pragma once

#include <SDL2/SDL.h>
#include <string>

#include "game/ScoreStorage.h"
#include "game/State.h"
#include "game/StickmanSkeleton.h"

class MenuState : public State {
public:
    MenuState();

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

    float elapsed_ = 0.0f;
    int best_score_ = 0;
    float title_x_ = -400.0f;
    float buttons_y_ = 1100.0f;
    float score_alpha_ = 1.0f;
    bool start_transition_ = false;
    float start_transition_timer_ = 0.0f;
    int land_index_ = 0;
    float score_color_timer_ = 0.0f;
    SDL_Color score_color_{0, 0, 0, 255};
    StickmanSkeleton stickman_{};
    bool stickman_loaded_ = false;

    Button gamecenter_{};
    Button share_{};

    ScoreStorage storage_;

    bool IsInside(const Button& button, float x, float y) const;
};
