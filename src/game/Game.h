#pragma once

#include <SDL2/SDL.h>
#include <memory>

#include "Assets.h"
#include "RenderContext.h"
#include "State.h"

class Game {
public:
    Game();
    ~Game();

    bool Init();
    void Run();
    void Shutdown();

    void ChangeState(std::unique_ptr<State> next_state);

    SDL_Renderer* Renderer() { return renderer_; }
    SDL_Window* Window() { return window_; }
    Assets& GetAssets() { return assets_; }
    const RenderContext& RenderCtx() const { return render_ctx_; }
    RenderContext& RenderCtx() { return render_ctx_; }

    void Quit() { running_ = false; }

private:
    void ProcessEvents();

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    bool running_ = true;

    Assets assets_{};
    RenderContext render_ctx_{};
    std::unique_ptr<State> state_;
};
