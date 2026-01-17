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
}

void MenuState::Exit(Game& game) {
    (void)game;
}

bool MenuState::IsInside(const Button& button, float x, float y) const {
    return x >= button.x - button.w * 0.5f && x <= button.x + button.w * 0.5f &&
           y >= button.y - button.h * 0.5f && y <= button.y + button.h * 0.5f;
}

void MenuState::HandleEvent(Game& game, const SDL_Event& event) {
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
        game.ChangeState(std::make_unique<GameState>(best_score_, land_index_));
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

    const float title_t = ClampFloat(elapsed_ / kTitleEnterDuration, 0.0f, 1.0f);
    title_x_ = Lerp(-400.0f, 1216.0f / 2.0f, EaseOutBounce(title_t));

    const float button_t = ClampFloat(elapsed_ / kButtonEnterDuration, 0.0f, 1.0f);
    buttons_y_ = Lerp(800.0f + 304.0f, kButtonYTarget, EaseOutBounce(button_t));
    gamecenter_.y = buttons_y_;
    share_.y = buttons_y_;
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

    const float touch_alpha = 0.5f + 0.5f * std::sin(elapsed_ * 4.0f);
    DrawTextureCentered(renderer, ctx, assets.GetTexture("touchToPlay"), 1216.0f / 2.0f, 800.0f - 404.0f, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255}, touch_alpha);

    DrawTextureCentered(renderer, ctx, assets.GetTexture("title"), title_x_, 104.0f, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    const TextureAsset word_best = assets.GetTexture("wordBest");
    const TextureAsset colon = assets.GetTexture("colon");
    const TextureAsset dot = assets.GetTexture("dot");
    const BitmapFont* score_font = assets.GetFont("numberScoreMain");

    float base_x = 1216.0f / 2.0f - 100.0f;
    float base_y = 800.0f / 2.0f - 100.0f;

    DrawTextureCentered(renderer, ctx, word_best, base_x, base_y, 1.3f, 1.3f, SDL_Color{255, 10, 99, 255});
    DrawTextureCentered(renderer, ctx, colon, base_x + 100.0f, base_y + 10.0f, 1.0f, 1.0f, SDL_Color{255, 10, 99, 255});

    if (score_font) {
        const int integer = best_score_ / 10;
        const int decimal = best_score_ % 10;
        int integer_width = 0;
        score_font->Draw(renderer, ctx, std::to_string(integer), base_x + 120.0f, base_y - 10.0f, 1.0f, SDL_Color{0, 0, 0, 255}, &integer_width);
        DrawTexture(renderer, ctx, dot, base_x + 120.0f + static_cast<float>(integer_width) + 2.0f, base_y + 20.0f, 1.0f, 1.0f,
                    SDL_Color{0, 0, 0, 255});
        score_font->Draw(renderer, ctx, std::to_string(decimal), base_x + 120.0f + static_cast<float>(integer_width) + 2.0f + static_cast<float>(dot.width),
                         base_y - 10.0f, 1.0f, SDL_Color{0, 0, 0, 255});
    }

    const std::string gamecenter_key = gamecenter_.key_base + std::string(gamecenter_.pressed ? "1" : "0");
    const std::string share_key = share_.key_base + std::string(share_.pressed ? "1" : "0");

    DrawTextureCentered(renderer, ctx, assets.GetTexture(gamecenter_key), gamecenter_.x, gamecenter_.y, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(share_key), share_.x, share_.y, 1.0f, 1.0f,
                        SDL_Color{255, 255, 255, 255});

    SDL_RenderPresent(renderer);
}
