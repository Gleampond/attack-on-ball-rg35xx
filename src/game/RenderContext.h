#pragma once

#include <SDL2/SDL.h>

#include "Constants.h"

struct RenderContext {
    float scale = 1.0f;
    int offset_x = 0;
    int offset_y = 0;

    void Update() {
        const float scale_x = static_cast<float>(constants::kScreenWidth) / static_cast<float>(constants::kDesignWidth);
        const float scale_y = static_cast<float>(constants::kScreenHeight) / static_cast<float>(constants::kDesignHeight);
        scale = (scale_x < scale_y) ? scale_x : scale_y;
        const int scaled_w = static_cast<int>(static_cast<float>(constants::kDesignWidth) * scale);
        const int scaled_h = static_cast<int>(static_cast<float>(constants::kDesignHeight) * scale);
        offset_x = (constants::kScreenWidth - scaled_w) / 2;
        offset_y = (constants::kScreenHeight - scaled_h) / 2;
    }

    SDL_FPoint ScreenToWorld(int x, int y) const {
        return SDL_FPoint{(static_cast<float>(x) - static_cast<float>(offset_x)) / scale,
                          (static_cast<float>(y) - static_cast<float>(offset_y)) / scale};
    }

    SDL_FRect WorldToScreenRect(const SDL_FRect& rect) const {
        return SDL_FRect{offset_x + rect.x * scale, offset_y + rect.y * scale,
                         rect.w * scale, rect.h * scale};
    }
};
