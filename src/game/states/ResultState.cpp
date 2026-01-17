#include "ResultState.h"

#include <string>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>

#include "game/Assets.h"
#include "game/Game.h"
#include "game/MathUtils.h"
#include "game/RenderHelpers.h"
#include "game/states/MenuState.h"

ResultState::ResultState(int best_score, int your_score, int land_index)
    : best_score_(best_score), your_score_(your_score), land_index_(land_index), storage_("save.dat") {}

void ResultState::Enter(Game& game) {
    elapsed_ = 0.0f;
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
        const int best_integer = best_score_ / 10;
        const int best_decimal = best_score_ % 10;
        int best_width = 0;
        score_font->Draw(renderer, ctx, std::to_string(best_integer), 1216.0f / 2.0f - 150.0f, 800.0f / 2.0f - 170.0f, 1.0f,
                         SDL_Color{0, 0, 0, 255}, &best_width);
        DrawTexture(renderer, ctx, assets.GetTexture("dot"), 1216.0f / 2.0f - 150.0f + static_cast<float>(best_width) + 2.0f,
                    800.0f / 2.0f - 170.0f + 20.0f, 1.0f, 1.0f, SDL_Color{0, 0, 0, 255});
        score_font->Draw(renderer, ctx, std::to_string(best_decimal), 1216.0f / 2.0f - 150.0f + static_cast<float>(best_width) + 20.0f,
                         800.0f / 2.0f - 170.0f, 1.0f, SDL_Color{0, 0, 0, 255});

        const int your_integer = your_score_ / 10;
        const int your_decimal = your_score_ % 10;
        int your_width = 0;
        score_font->Draw(renderer, ctx, std::to_string(your_integer), 1216.0f / 2.0f + 150.0f, 800.0f / 2.0f - 170.0f, 1.0f,
                         SDL_Color{0, 0, 0, 255}, &your_width);
        DrawTexture(renderer, ctx, assets.GetTexture("dot"), 1216.0f / 2.0f + 150.0f + static_cast<float>(your_width) + 2.0f,
                    800.0f / 2.0f - 170.0f + 20.0f, 1.0f, 1.0f, SDL_Color{0, 0, 0, 255});
        score_font->Draw(renderer, ctx, std::to_string(your_decimal), 1216.0f / 2.0f + 150.0f + static_cast<float>(your_width) + 20.0f,
                         800.0f / 2.0f - 170.0f, 1.0f, SDL_Color{0, 0, 0, 255});
    }

    DrawTextureCentered(renderer, ctx, assets.GetTexture(gamecenter_.key_base + std::string(gamecenter_.pressed ? "1" : "0")),
                        gamecenter_.x, gamecenter_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(share_.key_base + std::string(share_.pressed ? "1" : "0")),
                        share_.x, share_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});
    DrawTextureCentered(renderer, ctx, assets.GetTexture(play_.key_base + std::string(play_.pressed ? "1" : "0")),
                        play_.x, play_.y, 1.0f, 1.0f, SDL_Color{255, 255, 255, 255});

    SDL_RenderPresent(renderer);
}
