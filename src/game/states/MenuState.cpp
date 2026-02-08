#include "MenuState.h"

#include <cmath>
#include <random>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game/Assets.h"
#include "game/Game.h"
#include "game/MathUtils.h"
#include "game/RenderHelpers.h"
#include "game/states/GameState.h"

namespace {
constexpr float kButtonYTarget = 800.0f - 304.0f;
constexpr float kButtonEnterDuration = 0.5f;
constexpr float kTitleEnterDuration = 0.5f;
constexpr float kScoreColorInterval = 0.1f;
constexpr float kStartTransitionDuration = 0.35f;

int RandomInt(int min_value, int max_value) {
    static std::mt19937 rng{static_cast<unsigned int>(SDL_GetTicks())};
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(rng);
}
}

MenuState::MenuState() : storage_("save.dat") {}

void MenuState::Enter(Game& game) {
    elapsed_ = 0.0f;
    best_score_ = storage_.LoadBestScore();
    std::mt19937 rng{static_cast<unsigned int>(SDL_GetTicks())};
    std::uniform_int_distribution<int> land_dist(0, 5);
    land_index_ = land_dist(rng);

    const Assets& assets = game.GetAssets();
    const TextureAsset gamecenter = assets.GetTexture("buttonGamecenter0");
    const TextureAsset share = assets.GetTexture("buttonShare0");

    gamecenter_ = {120.0f, 800.0f + 304.0f, static_cast<float>(gamecenter.width), static_cast<float>(gamecenter.height), false, "buttonGamecenter"};
    share_ = {1216.0f - 120.0f, 800.0f + 304.0f, static_cast<float>(share.width), static_cast<float>(share.height), false, "buttonShare"};

    title_x_ = -400.0f;
    buttons_y_ = 800.0f + 304.0f;
    score_alpha_ = 1.0f;
    start_transition_ = false;
    start_transition_timer_ = 0.0f;
    score_color_timer_ = 0.0f;
    score_color_ = SDL_Color{0, 0, 0, 255};
    if (!stickman_loaded_) {
        stickman_loaded_ = stickman_.Load("assets/Stickman.json", "assets/Stickman.atlas");
    }
    if (stickman_loaded_) {
        stickman_.SetAnimation("Idle", true, true);
    }
}

void MenuState::Exit(Game& game) {
    (void)game;
}

bool MenuState::IsInside(const Button& button, float x, float y) const {
    return x >= button.x - button.w * 0.5f && x <= button.x + button.w * 0.5f &&
           y >= button.y - button.h * 0.5f && y <= button.y + button.h * 0.5f;
}

void MenuState::HandleEvent(Game& game, const SDL_Event& event) {
    if (start_transition_) {
        return;
    }
    if (event.type == SDL_MOUSEBUTTONDOWN) {
        const SDL_FPoint pos = game.RenderCtx().ScreenToWorld(event.button.x, event.button.y);
        if (IsInside(gamecenter_, pos.x, pos.y)) {
            gamecenter_.pressed = true;
            game.GetAssets().PlaySound("uiButton", MIX_MAX_VOLUME / 2);
            return;
        }
        if (IsInside(share_, pos.x, pos.y)) {
            share_.pressed = true;
            game.GetAssets().PlaySound("uiButton", MIX_MAX_VOLUME / 2);
            return;
        }
        if (!start_transition_) {
            start_transition_ = true;
            start_transition_timer_ = 0.0f;
            game.GetAssets().PlaySound("uiButton", MIX_MAX_VOLUME / 2);
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        const SDL_FPoint pos = game.RenderCtx().ScreenToWorld(event.button.x, event.button.y);
        if (gamecenter_.pressed && IsInside(gamecenter_, pos.x, pos.y)) {
            gamecenter_.pressed = false;
        }
        if (share_.pressed && IsInside(share_, pos.x, pos.y)) {
            share_.pressed = false;
        }
        gamecenter_.pressed = false;
        share_.pressed = false;
    }
}

void MenuState::Update(Game& game, float delta_seconds) {
    (void)game;
    elapsed_ += delta_seconds;

    if (start_transition_) {
        start_transition_timer_ += delta_seconds;
        const float t = ClampFloat(start_transition_timer_ / kStartTransitionDuration, 0.0f, 1.0f);
        const float ease = EaseOutCubic(t);
        title_x_ = Lerp(1216.0f / 2.0f, 1216.0f + 400.0f, ease);
        buttons_y_ = Lerp(kButtonYTarget, 800.0f + 304.0f, ease);
        score_alpha_ = Lerp(1.0f, 0.0f, ease);
        if (t >= 1.0f) {
            game.ChangeState(std::make_unique<GameState>(best_score_, land_index_));
            return;
        }
    } else {
        const float title_t = ClampFloat(elapsed_ / kTitleEnterDuration, 0.0f, 1.0f);
        title_x_ = Lerp(-400.0f, 1216.0f / 2.0f, EaseOutBounce(title_t));

        const float button_t = ClampFloat(elapsed_ / kButtonEnterDuration, 0.0f, 1.0f);
        buttons_y_ = Lerp(800.0f + 304.0f, kButtonYTarget, EaseOutBounce(button_t));
        score_alpha_ = 1.0f;
    }
    gamecenter_.y = buttons_y_;
    share_.y = buttons_y_;
    if (stickman_loaded_) {
        stickman_.Update(delta_seconds);
    }

    score_color_timer_ += delta_seconds;
    while (score_color_timer_ >= kScoreColorInterval) {
        score_color_timer_ -= kScoreColorInterval;
        score_color_ = SDL_Color{
            static_cast<Uint8>(RandomInt(0, 255)),
            static_cast<Uint8>(RandomInt(0, 255)),
            static_cast<Uint8>(RandomInt(0, 255)),
            255
        };
    }
}

void MenuState::Render(Game& game) {
    SDL_Renderer* renderer = game.Renderer();
    const RenderContext& ctx = game.RenderCtx();
    Assets& assets = game.GetAssets();

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    DrawTexture(renderer, ctx, assets.GetTexture("bg"), 0.0f, 0.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    DrawTexture(renderer, ctx, assets.GetTexture("land" + std::to_string(land_index_)), 0.0f, 800.0f - 204.0f, 1.0f, 1.0f,
                SDL_Color{255, 255, 255, 255});

    const float touch_alpha = (0.5f + 0.5f * std::sin(elapsed_ * 4.0f)) * score_alpha_;
    DrawTextureCentered(renderer, ctx, assets.GetTexture("touchToPlay"), 1216.0f / 2.0f, 800.0f - 404.0f, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255}, touch_alpha);

    DrawTextureCentered(renderer, ctx, assets.GetTexture("title"), title_x_, 104.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    const TextureAsset word_best = assets.GetTexture("wordBest");
    const TextureAsset colon = assets.GetTexture("colon");
    const TextureAsset dot = assets.GetTexture("dot");
    const BitmapFont* score_font = assets.GetFont("numberScoreMain");

    float base_x = 1216.0f / 2.0f - 100.0f;
    float base_y = 800.0f / 2.0f - 100.0f;

    SDL_Color score_label_color{255, 10, 99, static_cast<Uint8>(255.0f * score_alpha_)};
    DrawTextureCentered(renderer, ctx, word_best, base_x, base_y, 1.3f, 1.3f, score_label_color);
    DrawTextureCentered(renderer, ctx, colon, base_x + 100.0f, base_y + 10.0f, 1.0f, 1.0f, score_label_color);

    if (score_font) {
        const int integer = best_score_ / 10;
        const int decimal = best_score_ % 10;
        int integer_width = 0;
        SDL_Color score_color = score_color_;
        score_color.a = static_cast<Uint8>(255.0f * score_alpha_);
        score_font->Draw(renderer, ctx, std::to_string(integer), base_x + 120.0f, base_y - 10.0f, 1.0f, score_color, &integer_width);
        DrawTexture(renderer, ctx, dot, base_x + 120.0f + static_cast<float>(integer_width) + 2.0f, base_y + 20.0f, 1.0f, 1.0f,
                    score_color);
        score_font->Draw(renderer, ctx, std::to_string(decimal), base_x + 120.0f + static_cast<float>(integer_width) + 2.0f + static_cast<float>(dot.width),
                         base_y - 10.0f, 1.0f, score_color);
    }

    const std::string gamecenter_key = gamecenter_.key_base + std::string(gamecenter_.pressed ? "1" : "0");
    const std::string share_key = share_.key_base + std::string(share_.pressed ? "1" : "0");

    DrawTextureCentered(renderer, ctx, assets.GetTexture(gamecenter_key), gamecenter_.x, gamecenter_.y, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(share_key), share_.x, share_.y, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255});

    DrawTextureCentered(renderer, ctx, assets.GetTexture("shadow"), 1216.0f / 2.0f, 800.0f - 204.0f + 40.0f, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255});
    if (stickman_loaded_) {
        stickman_.Draw(renderer, ctx, assets.GetTexture("stickman"), 1216.0f / 2.0f, 800.0f - 204.0f + 20.0f, 1.5f,
                       SDL_Color{255, 255, 255, 255});
    }

    SDL_RenderPresent(renderer);
}
