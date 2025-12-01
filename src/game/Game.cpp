// Game loop implementation.
#include "Game.h"

#include "Constants.h"

Game::Game() = default;
Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
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

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        Shutdown();
        return false;
    }

    previous_ticks_ = SDL_GetTicks();
    return true;
}

void Game::Run() {
    while (is_running_) {
        const Uint32 current_ticks = SDL_GetTicks();
        const float delta_seconds = (current_ticks - previous_ticks_) / 1000.0f;
        previous_ticks_ = current_ticks;

        ProcessInput();
        Update(delta_seconds);
        Render();

        // Simple frame cap to avoid runaway CPU usage.
        const Uint32 frame_delay_ms = 1000 / constants::kTargetFps;
        const Uint32 frame_time = SDL_GetTicks() - current_ticks;
        if (frame_delay_ms > frame_time) {
            SDL_Delay(frame_delay_ms - frame_time);
        }
    }
}

void Game::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            is_running_ = false;
        }
    }

    const Uint8* keyboard_state = SDL_GetKeyboardState(nullptr);
    player_.HandleInput(keyboard_state);
}

void Game::Update(float delta_seconds) {
    player_.Update(delta_seconds);
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderClear(renderer_);

    player_.Render(renderer_);

    SDL_RenderPresent(renderer_);
}

void Game::Shutdown() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    SDL_Quit();
}
