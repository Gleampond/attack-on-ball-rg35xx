#include "GameState.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <string>

#include "game/Assets.h"
#include "game/Game.h"
#include "game/MathUtils.h"
#include "game/RenderHelpers.h"
#include "game/states/ResultState.h"

#include <SDL2/SDL_mixer.h>

namespace {
constexpr float kHeroSpeed = 400.0f;
constexpr float kGravity = 950.0f;
constexpr float kNumberGravity = 950.0f;
constexpr float kGroundY = 800.0f - 204.0f;
constexpr float kGroundHeight = 184.0f;
constexpr float kGroundOffset = 20.0f;
constexpr float kMoveThreshold = 1.0f;

const char* kRunAnimations[] = {"Run0", "Run1", "Run2", "Run3", "RunSmile"};
const char* kIdleAnimations[] = {"Idle", "IdleSmile"};

float RandomRange(float min_value, float max_value) {
    static std::mt19937 rng{static_cast<unsigned int>(SDL_GetTicks())};
    std::uniform_real_distribution<float> dist(min_value, max_value);
    return dist(rng);
}

int RandomInt(int min_value, int max_value) {
    static std::mt19937 rng{static_cast<unsigned int>(SDL_GetTicks())};
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(rng);
}

bool RectOverlap(const SDL_FRect& a, const SDL_FRect& b) {
    return !(a.x > b.x + b.w || a.x + a.w < b.x || a.y > b.y + b.h || a.y + a.h < b.y);
}

std::string PickRandomAnimation(const char* const* options, int count) {
    return options[RandomInt(0, count - 1)];
}
}

GameState::GameState(int best_score, int land_index)
    : best_score_(best_score), land_index_(land_index) {}

void GameState::Enter(Game& game) {
    elapsed_ = 0.0f;
    ball_timer_ = 0.0f;
    number_timer_ = 0.0f;
    gauge_timer_ = 0.0f;
    floor_timer_ = 0.0f;
    floor_flash_timer_ = 0.0f;
    floor_flashing_ = false;
    gauge_count_ = 0;
    left_ball_ = true;
    dead_ = false;
    dead_timer_ = 0.0f;
    effect_blood_frame_ = -1;
    effect_blood_timer_ = 0.0f;
    red_border_timer_ = 0.0f;
    shake_timer_ = 0.0f;

    balls_.clear();
    numbers_.clear();
    blood_particles_.clear();
    dead_parts_.clear();

    ResetHero();
    UpdateGauge();

    if (!stickman_loaded_) {
        stickman_loaded_ = stickman_.Load("assets/Stickman.json", "assets/Stickman.atlas");
    }
    was_moving_ = false;
    hero_animation_ = "Idle";
    hero_facing_left_ = false;
    if (stickman_loaded_) {
        stickman_.SetAnimation(hero_animation_, true, true);
    }
}

void GameState::Exit(Game& game) {
    (void)game;
}

void GameState::HandleEvent(Game& game, const SDL_Event& event) {
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        const SDL_FPoint pos = game.RenderCtx().ScreenToWorld(event.button.x, event.button.y);
        mouse_down_ = true;
        mouse_dir_ = (pos.x > 1216.0f / 2.0f) ? 1.0f : -1.0f;
        hero_facing_left_ = (mouse_dir_ < 0.0f);
        if (!dead_ && stickman_loaded_) {
            hero_animation_ = PickRandomAnimation(kRunAnimations, 5);
            stickman_.SetAnimation(hero_animation_, true, true);
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        mouse_down_ = false;
        mouse_dir_ = 0.0f;
        if (!dead_ && stickman_loaded_) {
            hero_animation_ = PickRandomAnimation(kIdleAnimations, 2);
            stickman_.SetAnimation(hero_animation_, true, true);
        }
    }
}

void GameState::Update(Game& game, float delta_seconds) {
    elapsed_ += delta_seconds;

    if (!dead_) {
        HandleInput(game, delta_seconds);
        const bool is_moving = std::abs(hero_.velocity.x) > kMoveThreshold;
        if (stickman_loaded_ && is_moving != was_moving_) {
            if (is_moving) {
                hero_animation_ = PickRandomAnimation(kRunAnimations, 5);
            } else {
                hero_animation_ = PickRandomAnimation(kIdleAnimations, 2);
            }
            stickman_.SetAnimation(hero_animation_, true, true);
        }
        was_moving_ = is_moving;
        if (stickman_loaded_) {
            stickman_.Update(delta_seconds);
        }
        UpdateHero(delta_seconds);
        UpdateBalls(delta_seconds);
        UpdateNumbers(game, delta_seconds);
        UpdateParticles(delta_seconds);

        CheckCollisions(game);

        ball_timer_ += delta_seconds;
        if (ball_timer_ >= 1.0f) {
            ball_timer_ = 0.0f;
            SpawnBall(game);
        }

        number_timer_ += delta_seconds;
        if (number_timer_ >= 5.0f) {
            number_timer_ = 0.0f;
            SpawnNumber(game);
        }

        gauge_timer_ += delta_seconds;
        if (gauge_timer_ >= 0.1f) {
            gauge_timer_ -= 0.1f;
            gauge_count_ += 1;
            UpdateGauge();
        }

        floor_timer_ += delta_seconds;
        if (floor_timer_ >= 10.0f) {
            floor_timer_ = 0.0f;
            floor_flashing_ = true;
            floor_flash_timer_ = 0.0f;
        }

        if (floor_flashing_) {
            floor_flash_timer_ += delta_seconds;
            if (floor_flash_timer_ >= 1.0f) {
                floor_flashing_ = false;
                land_index_ = RandomInt(0, 5);
            }
        }
    } else {
        dead_timer_ += delta_seconds;
        UpdateParticles(delta_seconds);
        if (effect_blood_frame_ >= 0) {
            effect_blood_timer_ += delta_seconds;
            if (effect_blood_timer_ >= 0.05f) {
                effect_blood_timer_ = 0.0f;
                effect_blood_frame_++;
                if (effect_blood_frame_ > 10) {
                    effect_blood_frame_ = -1;
                }
            }
        }
        if (shake_timer_ > 0.0f) {
            shake_timer_ -= delta_seconds;
        }
        if (red_border_timer_ > 0.0f) {
            red_border_timer_ -= delta_seconds;
        }
        if (dead_timer_ >= 2.0f) {
            game.ChangeState(std::make_unique<ResultState>(best_score_, gauge_count_, land_index_));
        }
    }
}

void GameState::Render(Game& game) {
    SDL_Renderer* renderer = game.Renderer();
    RenderContext ctx = game.RenderCtx();

    float shake_x = 0.0f;
    float shake_y = 0.0f;
    if (shake_timer_ > 0.0f) {
        shake_x = RandomRange(-10.0f, 10.0f);
        shake_y = RandomRange(-10.0f, 10.0f);
    }
    ctx.offset_x += static_cast<int>(shake_x * ctx.scale);
    ctx.offset_y += static_cast<int>(shake_y * ctx.scale);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    Assets& assets = game.GetAssets();
    DrawTexture(renderer, ctx, assets.GetTexture("bg"), 0.0f, 0.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    const std::string land_key = floor_flashing_ ? "landWhite" : ("land" + std::to_string(land_index_));
    SDL_Color land_tint = {255, 255, 255, 255};
    if (floor_flashing_) {
        land_tint = SDL_Color{static_cast<Uint8>(RandomInt(0, 255)), static_cast<Uint8>(RandomInt(0, 255)), static_cast<Uint8>(RandomInt(0, 255)), 255};
    }
    DrawTexture(renderer, ctx, assets.GetTexture(land_key), 0.0f, kGroundY, 1.0f, 1.0f, land_tint);

    for (const auto& ball : balls_) {
        if (!ball.alive) {
            continue;
        }
        const TextureAsset asset = assets.GetTexture("ball" + std::to_string(ball.texture_index));
        DrawTextureCentered(renderer, ctx, asset, ball.pos.x, ball.pos.y, ball.scale, ball.scale, SDL_Color{255, 255, 255, 255});

        const TextureAsset shadow = assets.GetTexture("shadow");
        float shadow_scale = 0.5f + (ball.pos.y / (800.0f - kGroundHeight + 20.0f)) / 2.0f;
        float shadow_alpha = 0.5f + (ball.pos.y / (800.0f - kGroundHeight + 20.0f)) / 2.0f;
        DrawTextureCentered(renderer, ctx, shadow, ball.pos.x, 800.0f - kGroundHeight + 20.0f, shadow_scale, shadow_scale, SDL_Color{255, 255, 255, 255}, shadow_alpha);
    }

    for (const auto& number : numbers_) {
        if (!number.alive) {
            continue;
        }
        const TextureAsset asset = assets.GetTexture("numberItem" + std::to_string(number.value));
        DrawTextureCentered(renderer, ctx, asset, number.pos.x, number.pos.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    }

    if (hero_.alive) {
        if (stickman_loaded_) {
            stickman_.Draw(renderer, ctx, assets.GetTexture("stickman"), hero_.pos.x, hero_.pos.y, 1.5f,
                           SDL_Color{255, 255, 255, 255}, hero_facing_left_);
        } else {
            DrawTextureCentered(renderer, ctx, assets.GetTexture("stickman"), hero_.pos.x, hero_.pos.y, 1.5f, 1.5f, SDL_Color{255, 255, 255, 255});
        }
        DrawTextureCentered(renderer, ctx, assets.GetTexture("shadow"), hero_.pos.x, hero_.pos.y + 20.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    }

    for (const auto& particle : blood_particles_) {
        const TextureAsset asset = assets.GetTexture("blood");
        DrawTextureCentered(renderer, ctx, asset, particle.pos.x, particle.pos.y, particle.scale, particle.scale, particle.color);
    }

    for (const auto& part : dead_parts_) {
        const TextureAsset asset = assets.GetTexture("deadParts" + std::to_string(part.texture_index));
        DrawTextureCentered(renderer, ctx, asset, part.pos.x, part.pos.y, part.scale, part.scale, part.color);
    }

    if (effect_blood_frame_ >= 0) {
        const TextureAsset asset = assets.GetTexture("effectBlood" + std::to_string(effect_blood_frame_));
        DrawTextureCentered(renderer, ctx, asset, hero_.pos.x, hero_.pos.y - 45.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    }

    const TextureAsset gauge = assets.GetTexture("gauge");
    const TextureAsset gauge_head = assets.GetTexture("gaugeHead");

    const float gauge_scale = 1216.0f / static_cast<float>(gauge.width);
    const int crop_width = std::min(gauge.width, static_cast<int>(gauge_head_x_ / gauge_scale));
    SDL_Rect src{0, 0, crop_width, gauge.height};
    DrawTextureSubrect(renderer, ctx, gauge, src, 0.0f, 0.0f, gauge_scale, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTexture(renderer, ctx, gauge_head, gauge_head_x_ - 10.0f, 0.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    const BitmapFont* time_font = assets.GetFont("numberTime");
    if (time_font) {
        const int integer = gauge_count_ / 10;
        const int decimal = gauge_count_ % 10;
        int integer_width = 0;
        const float time_y = static_cast<float>(gauge_head.height);
        time_font->Draw(renderer, ctx, std::to_string(integer), gauge_head_x_ - 30.0f, time_y, 1.0f, SDL_Color{0, 0, 0, 255}, &integer_width);
        const TextureAsset dot = assets.GetTexture("white4");
        DrawTexture(renderer, ctx, dot, gauge_head_x_ - 30.0f + static_cast<float>(integer_width) + 2.0f,
                    time_y + static_cast<float>(time_font->LineHeight()) - 5.0f, 1.0f, 1.0f, SDL_Color{0, 0, 0, 255});
        time_font->Draw(renderer, ctx, std::to_string(decimal), gauge_head_x_ - 30.0f + static_cast<float>(integer_width) + 2.0f + static_cast<float>(dot.width),
                        time_y, 1.0f, SDL_Color{0, 0, 0, 255});
    }

    if (red_border_timer_ > 0.0f) {
        const TextureAsset hit = assets.GetTexture("effectHit");
        DrawTexture(renderer, ctx, hit, 0.0f, 0.0f, 1216.0f / static_cast<float>(hit.width), 800.0f / static_cast<float>(hit.height),
                    SDL_Color{255, 255, 255, 255}, 0.8f);
    }

    SDL_RenderPresent(renderer);
}

void GameState::ResetHero() {
    hero_.pos = SDL_FPoint{1216.0f / 2.0f, kGroundY + 20.0f};
    hero_.velocity = SDL_FPoint{0.0f, 0.0f};
    hero_.dir = 0.0f;
    hero_.alive = true;
}

void GameState::HandleInput(Game& game, float delta_seconds) {
    (void)delta_seconds;
    const Uint8* keys = SDL_GetKeyboardState(nullptr);
    float dir = 0.0f;
    if (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]) {
        dir -= 1.0f;
    }
    if (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]) {
        dir += 1.0f;
    }
    if (mouse_down_) {
        dir = mouse_dir_;
    }
    hero_.dir = dir;
    hero_.velocity.x = hero_.dir * kHeroSpeed;
    if (hero_.dir < 0.0f) {
        hero_facing_left_ = true;
    } else if (hero_.dir > 0.0f) {
        hero_facing_left_ = false;
    }
}

void GameState::UpdateHero(float delta_seconds) {
    hero_.pos.x += hero_.velocity.x * delta_seconds;
    hero_.pos.x = ClampFloat(hero_.pos.x, 0.0f, 1216.0f);
}

void GameState::UpdateBalls(float delta_seconds) {
    const SDL_FRect ground = LandRect();
    for (auto& ball : balls_) {
        if (!ball.alive) {
            continue;
        }
        ball.velocity.y += kGravity * delta_seconds;
        ball.pos.x += ball.velocity.x * delta_seconds;
        ball.pos.y += ball.velocity.y * delta_seconds;

        const float ground_y = ground.y + ground.h - kGroundOffset;
        if (ball.pos.y + ball.radius >= ground_y) {
            ball.pos.y = ground_y - ball.radius;
            ball.velocity.y = -std::abs(ball.velocity.y);
        }

        if (ball.pos.x < -200.0f || ball.pos.x > 1216.0f + 200.0f) {
            ball.alive = false;
        }
    }
    balls_.erase(std::remove_if(balls_.begin(), balls_.end(), [](const Ball& ball) { return !ball.alive; }), balls_.end());
}

void GameState::UpdateNumbers(Game& game, float delta_seconds) {
    for (auto& number : numbers_) {
        if (!number.alive) {
            continue;
        }
        if (!number.collecting) {
            number.velocity.y += kNumberGravity * delta_seconds;
            number.pos.x += number.velocity.x * delta_seconds;
            number.pos.y += number.velocity.y * delta_seconds;
            if (number.pos.y > kGroundY) {
                number.pos.y = kGroundY;
                number.velocity.y = -number.velocity.y * 0.2f;
            }
        } else {
            const float dx = gauge_head_x_ - number.pos.x;
            const float dy = 10.0f - number.pos.y;
            const float dist = std::sqrt(dx * dx + dy * dy);
            const float speed = 1000.0f;
            if (dist < 5.0f) {
                number.alive = false;
                gauge_count_ += number.value * 10;
                UpdateGauge();
                game.GetAssets().PlaySound("numberGet", MIX_MAX_VOLUME / 2);
            } else {
                const float inv_dist = 1.0f / dist;
                number.pos.x += dx * inv_dist * speed * delta_seconds;
                number.pos.y += dy * inv_dist * speed * delta_seconds;
            }
        }
    }
    numbers_.erase(std::remove_if(numbers_.begin(), numbers_.end(), [](const NumberItem& item) { return !item.alive; }), numbers_.end());
}

void GameState::UpdateParticles(float delta_seconds) {
    for (auto& particle : blood_particles_) {
        particle.life -= delta_seconds;
        particle.velocity.y += kGravity * delta_seconds;
        particle.pos.x += particle.velocity.x * delta_seconds;
        particle.pos.y += particle.velocity.y * delta_seconds;
    }
    blood_particles_.erase(std::remove_if(blood_particles_.begin(), blood_particles_.end(),
                                          [](const Particle& p) { return p.life <= 0.0f; }),
                           blood_particles_.end());

    for (auto& part : dead_parts_) {
        part.life -= delta_seconds;
        part.velocity.y += kGravity * delta_seconds;
        part.pos.x += part.velocity.x * delta_seconds;
        part.pos.y += part.velocity.y * delta_seconds;
    }
    dead_parts_.erase(std::remove_if(dead_parts_.begin(), dead_parts_.end(),
                                     [](const Particle& p) { return p.life <= 0.0f; }),
                      dead_parts_.end());
}

void GameState::CheckCollisions(Game& game) {
    if (!hero_.alive) {
        return;
    }
    const SDL_FRect hero_rect = HeroRect();
    for (auto& ball : balls_) {
        if (!ball.alive) {
            continue;
        }
        SDL_FRect ball_rect{ball.pos.x - ball.radius, ball.pos.y - ball.radius, ball.radius * 2.0f, ball.radius * 2.0f};
        if (RectOverlap(hero_rect, ball_rect)) {
            OnDeath(game);
            return;
        }
    }
    for (auto& number : numbers_) {
        if (!number.alive) {
            continue;
        }
        SDL_FRect number_rect{number.pos.x - 20.0f, number.pos.y - 20.0f, 40.0f, 40.0f};
        if (RectOverlap(hero_rect, number_rect)) {
            number.collecting = true;
        }
    }
}

void GameState::OnDeath(Game& game) {
    hero_.alive = false;
    dead_ = true;

    StartEffects();
    game.GetAssets().PlaySound("die00", MIX_MAX_VOLUME);
}

void GameState::StartEffects() {
    effect_blood_frame_ = 0;
    effect_blood_timer_ = 0.0f;
    red_border_timer_ = 1.0f;
    shake_timer_ = 0.6f;

    blood_particles_.clear();
    for (int i = 0; i < 50; ++i) {
        Particle particle;
        particle.pos = SDL_FPoint{hero_.pos.x, hero_.pos.y - 45.0f};
        particle.velocity = SDL_FPoint{RandomRange(-300.0f, 300.0f), RandomRange(-1000.0f, 0.0f)};
        particle.life = RandomRange(0.8f, 1.5f);
        particle.scale = RandomRange(0.2f, 1.5f);
        particle.color = SDL_Color{255, 255, 255, 255};
        blood_particles_.push_back(particle);
    }

    dead_parts_.clear();
    for (int i = 0; i < 8; ++i) {
        Particle part;
        part.pos = SDL_FPoint{hero_.pos.x, hero_.pos.y - 45.0f};
        part.velocity = SDL_FPoint{RandomRange(-800.0f, 800.0f), RandomRange(-1000.0f, 0.0f)};
        part.life = RandomRange(1.0f, 2.0f);
        part.scale = 1.0f;
        part.texture_index = RandomInt(0, 7);
        dead_parts_.push_back(part);
    }
}

void GameState::SpawnBall(Game& game) {
    Ball ball;
    ball.texture_index = RandomInt(0, 4);
    const float scale = RandomRange(1.2f, 1.5f);
    ball.scale = scale;
    if (left_ball_) {
        ball.pos.x = -70.0f;
        ball.velocity.x = RandomRange(150.0f, 200.0f);
    } else {
        ball.pos.x = 1216.0f + 70.0f;
        ball.velocity.x = -RandomRange(150.0f, 200.0f);
    }
    left_ball_ = !left_ball_;
    ball.pos.y = RandomRange(-50.0f, 150.0f);
    ball.velocity.y = 0.0f;

    const TextureAsset asset = game.GetAssets().GetTexture("ball" + std::to_string(ball.texture_index));
    ball.radius = (static_cast<float>(asset.width) * scale) * 0.5f;
    ball.alive = true;
    balls_.push_back(ball);

    game.GetAssets().PlaySound("toss", MIX_MAX_VOLUME / 3);
}

void GameState::SpawnNumber(Game& game) {
    (void)game;
    NumberItem number;
    number.value = RandomInt(1, 4);
    number.pos = SDL_FPoint{RandomRange(0.0f, 1216.0f), -20.0f};
    number.velocity = SDL_FPoint{0.0f, 0.0f};
    number.angle = RandomRange(0.0f, 360.0f);
    number.alive = true;
    number.collecting = false;
    numbers_.push_back(number);
}

void GameState::UpdateGauge() {
    const float progress = static_cast<float>((gauge_count_ % 100) + 1) / 100.0f;
    gauge_head_x_ = progress * (1216.0f - 80.0f);
}

SDL_FRect GameState::LandRect() const {
    return SDL_FRect{0.0f, kGroundY, 1216.0f, kGroundHeight};
}

SDL_FRect GameState::HeroRect() const {
    return SDL_FRect{hero_.pos.x - 40.0f, hero_.pos.y - 80.0f, hero_.body.w, hero_.body.h};
}
