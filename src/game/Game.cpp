#include "Game.h"

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include "Constants.h"
#include "states/BootState.h"

Game::Game() = default;

Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        Shutdown();
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        Shutdown();
        return false;
    }

    window_ = SDL_CreateWindow(
        "Attack On Ball",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        constants::kScreenWidth,
        constants::kScreenHeight,
        SDL_WINDOW_SHOWN);

    if (!window_) {
        Shutdown();
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer_) {
        Shutdown();
        return false;
    }

    render_ctx_.Update();
    if (!assets_.Init(renderer_)) {
        Shutdown();
        return false;
    }

    ChangeState(std::make_unique<BootState>());
    return true;
}

void Game::Run() {
    Uint32 previous_ticks = SDL_GetTicks();
    const float target_frame_time = 1.0f / static_cast<float>(constants::kTargetFps);

    while (running_) {
        const Uint32 current_ticks = SDL_GetTicks();
        float delta_seconds = static_cast<float>(current_ticks - previous_ticks) / 1000.0f;
        if (delta_seconds > 0.05f) {
            delta_seconds = 0.05f;
        }
        previous_ticks = current_ticks;

        ProcessEvents();

        if (state_) {
            state_->Update(*this, delta_seconds);
            state_->Render(*this);
        }

        const Uint32 frame_time = SDL_GetTicks() - current_ticks;
        if (frame_time < static_cast<Uint32>(target_frame_time * 1000.0f)) {
            SDL_Delay(static_cast<Uint32>(target_frame_time * 1000.0f) - frame_time);
        }
    }
}

void Game::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running_ = false;
            return;
        }
        if (state_) {
            state_->HandleEvent(*this, event);
        }
    }
}

void Game::ChangeState(std::unique_ptr<State> next_state) {
    if (state_) {
        state_->Exit(*this);
    }
    state_ = std::move(next_state);
    if (state_) {
        state_->Enter(*this);
    }
}

void Game::Shutdown() {
    if (state_) {
        state_->Exit(*this);
        state_.reset();
    }
    assets_.Shutdown();
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    Mix_CloseAudio();
    IMG_Quit();
    SDL_Quit();
}
