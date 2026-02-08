#include "ResultState.h"

#include <cstdio>
#include <random>
#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game/Assets.h"
#include "game/Game.h"
#include "game/MathUtils.h"
#include "game/RenderHelpers.h"
#include "game/states/MenuState.h"

namespace {
int RandomInt(int min_value, int max_value) {
    static std::mt19937 rng{static_cast<unsigned int>(SDL_GetTicks())};
    std::uniform_int_distribution<int> dist(min_value, max_value);
    return dist(rng);
}
}

ResultState::ResultState(int best_score, int your_score, int land_index)
    : best_score_(best_score), your_score_(your_score), land_index_(land_index), storage_("save.dat") {}

void ResultState::Enter(Game& game) {
    elapsed_ = 0.0f;
    your_flash_timer_ = 0.0f;
    your_flash_color_ = SDL_Color{0, 0, 0, 255};
    if (your_score_ > best_score_) {
        best_score_ = your_score_;
        storage_.SaveBestScore(best_score_);
    }

    const Assets& assets = game.GetAssets();
    const TextureAsset gamecenter = assets.GetTexture("buttonGamecenter0");
    const TextureAsset share = assets.GetTexture("buttonShare0");
    const TextureAsset play = assets.GetTexture("buttonPlay0");

    gamecenter_ = {120.0f, 800.0f + 304.0f, static_cast<float>(gamecenter.width), static_cast<float>(gamecenter.height), false, "buttonGamecenter"};
    share_ = {1216.0f - 120.0f, 800.0f + 304.0f, static_cast<float>(share.width), static_cast<float>(share.height), false, "buttonShare"};
    play_ = {1216.0f / 2.0f, 800.0f + 284.0f, static_cast<float>(play.width), static_cast<float>(play.height), false, "buttonPlay"};
}

void ResultState::Exit(Game& game) {
    (void)game;
}

bool ResultState::IsInside(const Button& button, float x, float y) const {
    return x >= button.x - button.w * 0.5f && x <= button.x + button.w * 0.5f &&
           y >= button.y - button.h * 0.5f && y <= button.y + button.h * 0.5f;
}

void ResultState::HandleEvent(Game& game, const SDL_Event& event) {
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
        if (IsInside(play_, pos.x, pos.y)) {
            play_.pressed = true;
            game.GetAssets().PlaySound("uiButton", MIX_MAX_VOLUME / 2);
            return;
        }
    }
    if (event.type == SDL_MOUSEBUTTONUP) {
        const SDL_FPoint pos = game.RenderCtx().ScreenToWorld(event.button.x, event.button.y);
        if (play_.pressed && IsInside(play_, pos.x, pos.y)) {
            game.ChangeState(std::make_unique<MenuState>());
            return;
        }
        gamecenter_.pressed = false;
        share_.pressed = false;
        play_.pressed = false;
    }
}

void ResultState::Update(Game& game, float delta_seconds) {
    (void)game;
    elapsed_ += delta_seconds;
    your_flash_timer_ += delta_seconds;
    if (your_flash_timer_ >= 0.1f) {
        your_flash_timer_ -= 0.1f;
        your_flash_color_ = SDL_Color{
            static_cast<Uint8>(RandomInt(0, 255)),
            static_cast<Uint8>(RandomInt(0, 255)),
            static_cast<Uint8>(RandomInt(0, 255)),
            255
        };
    }

    const float t = ClampFloat(elapsed_ / 0.5f, 0.0f, 1.0f);
    const float ease = EaseOutBounce(t);
    gamecenter_.y = Lerp(800.0f + 304.0f, 800.0f - 304.0f, ease);
    share_.y = gamecenter_.y;
    play_.y = Lerp(800.0f + 284.0f, 800.0f - 304.0f, ease);
}

void ResultState::Render(Game& game) {
    SDL_Renderer* renderer = game.Renderer();
    const RenderContext& ctx = game.RenderCtx();
    Assets& assets = game.GetAssets();

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    DrawTexture(renderer, ctx, assets.GetTexture("bg"), 0.0f, 0.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTexture(renderer, ctx, assets.GetTexture("land" + std::to_string(land_index_)), 0.0f, 800.0f - 204.0f, 1.0f, 1.0f,
                SDL_Color{255, 255, 255, 255});

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 200);
    SDL_FRect overlay = ctx.WorldToScreenRect(SDL_FRect{0.0f, 0.0f, 1216.0f, 800.0f});
    SDL_RenderFillRectF(renderer, &overlay);

    const BitmapFont* score_font = assets.GetFont("numberScoreEnd");
    if (score_font) {
        const float group_y = 800.0f / 2.0f - 200.0f;
        const TextureAsset word_best = assets.GetTexture("wordBest");
        const TextureAsset word_your = assets.GetTexture("wordYour");
        const TextureAsset dot = assets.GetTexture("dot");

        const auto score_width = [&](int score) {
            const int integer_digits = static_cast<int>(std::to_string(score / 10).size());
            const int decimal_digits = 2;
            // NumberScoreEnd glyphs are fixed-width (55 advance in XML).
            return static_cast<float>(integer_digits * 55 + 2 + dot.width + decimal_digits * 55);
        };

        const float best_score_width = score_width(best_score_);
        const float your_score_width = score_width(your_score_);
        const float best_group_x = 1216.0f / 2.0f - 150.0f - best_score_width;
        const float your_group_x = 1216.0f / 2.0f + 150.0f;
        const float score_y = group_y + static_cast<float>(word_best.height) + 30.0f;

        DrawTexture(renderer, ctx, word_best, best_group_x + (best_score_width - static_cast<float>(word_best.width)) * 0.5f,
                    group_y, 1.0f, 1.0f, SDL_Color{0, 0, 0, 255});
        DrawTexture(renderer, ctx, word_your, your_group_x + (your_score_width - static_cast<float>(word_your.width)) * 0.5f,
                    group_y, 1.0f, 1.0f, SDL_Color{42, 216, 216, 255});

        const auto draw_score = [&](float x, float y, int score, SDL_Color color) {
            const int integer = score / 10;
            const int decimal_two = (score % 10) * 10;
            char decimal_text[3];
            std::snprintf(decimal_text, sizeof(decimal_text), "%02d", decimal_two);
            int integer_width = 0;
            score_font->Draw(renderer, ctx, std::to_string(integer), x, y, 1.0f, color, &integer_width);
            DrawTexture(renderer, ctx, dot, x + static_cast<float>(integer_width) + 2.0f, y + 20.0f, 1.0f, 1.0f, color);
            score_font->Draw(renderer, ctx, decimal_text, x + static_cast<float>(integer_width) + 2.0f + static_cast<float>(dot.width),
                             y, 1.0f, color);
        };

        draw_score(best_group_x, score_y, best_score_, SDL_Color{0, 0, 0, 255});
        draw_score(your_group_x, score_y, your_score_, your_flash_color_);
    }

    DrawTextureCentered(renderer, ctx, assets.GetTexture(gamecenter_.key_base + std::string(gamecenter_.pressed ? "1" : "0")),
                        gamecenter_.x, gamecenter_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(share_.key_base + std::string(share_.pressed ? "1" : "0")),
                        share_.x, share_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(play_.key_base + std::string(play_.pressed ? "1" : "0")),
                        play_.x, play_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    SDL_RenderPresent(renderer);
}
