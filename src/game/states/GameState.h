#pragma once

#include <SDL2/SDL.h>
#include <string>
#include <vector>

#include "game/ScoreStorage.h"
#include "game/State.h"
#include "game/StickmanSkeleton.h"

class GameState : public State {
public:
    GameState(int best_score, int land_index);

    void Enter(Game& game) override;
    void Exit(Game& game) override;
    void HandleEvent(Game& game, const SDL_Event& event) override;
    void Update(Game& game, float delta_seconds) override;
    void Render(Game& game) override;

private:
    struct Hero {
        SDL_FPoint pos{0.0f, 0.0f};
        SDL_FPoint velocity{0.0f, 0.0f};
        float dir = 0.0f;
        bool alive = true;
        SDL_FRect body{0.0f, 0.0f, 80.0f, 80.0f};
    };

    struct Ball {
        SDL_FPoint pos{0.0f, 0.0f};
        SDL_FPoint velocity{0.0f, 0.0f};
        float scale = 1.0f;
        float radius = 40.0f;
        int texture_index = 0;
        bool alive = true;
    };

    struct NumberItem {
        SDL_FPoint pos{0.0f, 0.0f};
        SDL_FPoint velocity{0.0f, 0.0f};
        float angle = 0.0f;
        int value = 1;
        bool alive = true;
        bool collecting = false;
        float flash_timer = 0.0f;
        SDL_Color tint{255, 255, 255, 255};
    };

    struct Particle {
        SDL_FPoint pos{0.0f, 0.0f};
        SDL_FPoint velocity{0.0f, 0.0f};
        float life = 0.0f;
        float scale = 1.0f;
        int texture_index = 0;
        SDL_Color color{255, 255, 255, 255};
    };

    struct Button {
        float x = 0.0f;
        float y = 0.0f;
        float w = 0.0f;
        float h = 0.0f;
        bool pressed = false;
        std::string key_base;
    };

    int best_score_ = 0;
    int land_index_ = 0;
    bool left_ball_ = true;

    Hero hero_{};
    std::vector<Ball> balls_;
    std::vector<NumberItem> numbers_;
    std::vector<Particle> blood_particles_;
    std::vector<Particle> dead_parts_;

    float elapsed_ = 0.0f;
    float ball_timer_ = 0.0f;
    float number_timer_ = 0.0f;
    float gauge_timer_ = 0.0f;
    float floor_timer_ = 0.0f;
    float floor_flash_timer_ = 0.0f;
    float floor_flash_color_timer_ = 0.0f;
    bool floor_flashing_ = false;
    SDL_Color floor_flash_tint_{255, 255, 255, 255};

    int gauge_count_ = 0;
    float gauge_head_x_ = 0.0f;
    float gauge_head_target_x_ = 0.0f;
    SDL_Color gauge_tint_{255, 255, 255, 255};
    SDL_Color gauge_head_tint_{255, 255, 255, 255};
    int gauge_flash_ticks_ = 0;

    bool mouse_down_ = false;
    float mouse_dir_ = 0.0f;

    bool dead_ = false;
    float dead_timer_ = 0.0f;
    int effect_blood_frame_ = -1;
    float effect_blood_timer_ = 0.0f;
    float red_border_timer_ = 0.0f;
    float shake_timer_ = 0.0f;
    StickmanSkeleton stickman_;
    bool stickman_loaded_ = false;
    bool was_moving_ = false;
    std::string hero_animation_;
    bool hero_facing_left_ = false;
    bool result_overlay_active_ = false;
    bool result_best_updated_ = false;
    float result_elapsed_ = 0.0f;
    float your_flash_timer_ = 0.0f;
    SDL_Color your_flash_color_{0, 0, 0, 255};
    Button result_gamecenter_{};
    Button result_share_{};
    Button result_play_{};
    ScoreStorage storage_{"save.dat"};

    void ResetHero();
    void SpawnBall(Game& game);
    void SpawnNumber(Game& game);
    void UpdateGauge();
    void HandleInput(Game& game, float delta_seconds);
    void UpdateHero(float delta_seconds);
    void UpdateBalls(float delta_seconds);
    void UpdateNumbers(Game& game, float delta_seconds);
    void UpdateParticles(float delta_seconds);

    void CheckCollisions(Game& game);
    void OnDeath(Game& game);
    void StartEffects();
    void StartResultOverlay(Game& game);
    bool IsInsideButton(const Button& button, float x, float y) const;
    void UpdateResultOverlay(float delta_seconds);
    void RenderResultOverlay(Game& game, const RenderContext& ctx);

    SDL_FRect LandRect() const;
    SDL_FRect HeroRect() const;
};
