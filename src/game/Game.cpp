// Game loop implementation.
#include "Game.h"

#include <SDL2/SDL_image.h>
#include <array>
#include <cstdio>
#include <string>

#include "Constants.h"

namespace {
void DrawSegment(SDL_Renderer* renderer, int x, int y, int w, int h) {
    SDL_Rect rect{x, y, w, h};
    SDL_RenderFillRect(renderer, &rect);
}

void DrawDigit(SDL_Renderer* renderer, char digit, int x, int y, int length, int thickness) {
    constexpr std::array<int, 10> kSegments = {
        0b1110111,  // 0
        0b0010010,  // 1
        0b1011101,  // 2
        0b1011011,  // 3
        0b0111010,  // 4
        0b1101011,  // 5
        0b1101111,  // 6
        0b1010010,  // 7
        0b1111111,  // 8
        0b1111011   // 9
    };

    if (digit < '0' || digit > '9') {
        return;
    }
    const int mask = kSegments[digit - '0'];
    const int vertical = length;
    const int horizontal = length + thickness;

    if (mask & 0b1000000) DrawSegment(renderer, x + thickness, y, horizontal, thickness);
    if (mask & 0b0100000) DrawSegment(renderer, x, y + thickness, thickness, vertical);
    if (mask & 0b0010000) DrawSegment(renderer, x + horizontal + thickness, y + thickness, thickness, vertical);
    if (mask & 0b0001000) DrawSegment(renderer, x + thickness, y + vertical + thickness, horizontal, thickness);
    if (mask & 0b0000100) DrawSegment(renderer, x, y + vertical + 2 * thickness, thickness, vertical);
    if (mask & 0b0000010) DrawSegment(renderer, x + horizontal + thickness, y + vertical + 2 * thickness, thickness, vertical);
    if (mask & 0b0000001) DrawSegment(renderer, x + thickness, y + 2 * vertical + 2 * thickness, horizontal, thickness);
}

int DigitWidth(int length, int thickness) {
    return length + thickness * 2 + length;
}

int DigitHeight(int length, int thickness) {
    return (length * 2) + (thickness * 3);
}

void RenderTimer(SDL_Renderer* renderer, float elapsed_seconds) {
    const int centi = static_cast<int>(elapsed_seconds * 100.0f);
    const int seconds = centi / 100;
    const int hundredths = centi % 100;

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d.%02d", seconds, hundredths);
    const std::string text(buffer);

    const int length = 10;
    const int thickness = 3;
    const int spacing = 4;
    const int dot_size = thickness;
    const int dot_offset = DigitHeight(length, thickness) - dot_size;

    int total_width = 0;
    for (char ch : text) {
        if (ch == '.') {
            total_width += dot_size + spacing;
        } else {
            total_width += DigitWidth(length, thickness) + spacing;
        }
    }
    if (total_width > 0) {
        total_width -= spacing;
    }

    const int start_x = (constants::kScreenWidth - total_width) / 2;
    const int start_y = 8;

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    int cursor_x = start_x;
    for (char ch : text) {
        if (ch == '.') {
            DrawSegment(renderer, cursor_x, start_y + dot_offset, dot_size, dot_size);
            cursor_x += dot_size + spacing;
            continue;
        }
        DrawDigit(renderer, ch, cursor_x, start_y, length, thickness);
        cursor_x += DigitWidth(length, thickness) + spacing;
    }
}
}  // namespace

Game::Game() = default;
Game::~Game() {
    Shutdown();
}

bool Game::Init() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        return false;
    }

    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
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

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_SOFTWARE);
    if (!renderer_) {
        Shutdown();
        return false;
    }

    previous_ticks_ = SDL_GetTicks();
    if (!player_.LoadTexture(renderer_, "assets/characters/ghost.png")) {
        Shutdown();
        return false;
    }
    if (!ball_.LoadTexture(renderer_, "assets/balls/ball_green.png")) {
        Shutdown();
        return false;
    }
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
    elapsed_time_ += delta_seconds;
    player_.Update(delta_seconds);
    ball_.Update(delta_seconds);
}

void Game::Render() {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderClear(renderer_);

    SDL_Rect ground{0, constants::kGroundTop, constants::kScreenWidth, constants::kGroundHeight};
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer_, &ground);

    player_.Render(renderer_);
    ball_.Render(renderer_);
    RenderTimer(renderer_, elapsed_time_);

    SDL_RenderPresent(renderer_);
}

void Game::Shutdown() {
    ball_.UnloadTexture();
    player_.UnloadTexture();
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
        renderer_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    IMG_Quit();
    SDL_Quit();
}
