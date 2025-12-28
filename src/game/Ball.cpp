// Implementation of a simple bouncing ball.
#include "Ball.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <cmath>
#include <string>

#include "Constants.h"

namespace {
void DrawFilledCircle(SDL_Renderer* renderer, int32_t center_x, int32_t center_y, int32_t radius) {
    for (int dy = -radius; dy <= radius; ++dy) {
        const int dx_limit = static_cast<int>(std::sqrt(radius * radius - dy * dy));
        SDL_RenderDrawLine(renderer, center_x - dx_limit, center_y + dy, center_x + dx_limit, center_y + dy);
    }
}
}  // namespace

Ball::Ball() {
    velocity_y_ = -bounce_speed_;
    ResetToGround();
}

Ball::~Ball() {
    FreeTexture();
}

bool Ball::LoadTexture(SDL_Renderer* renderer, const std::string& path) {
    FreeTexture();

    SDL_Surface* surface = IMG_Load(path.c_str());
    if (!surface) {
        return false;
    }

    texture_ = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture_) {
        SDL_FreeSurface(surface);
        return false;
    }

    texture_width_ = surface->w;
    texture_height_ = surface->h;
    const int min_dimension = (texture_width_ < texture_height_) ? texture_width_ : texture_height_;
    radius_ = static_cast<float>(min_dimension) * 0.5f * texture_scale_;
    ResetToGround();

    SDL_FreeSurface(surface);
    return true;
}

void Ball::UnloadTexture() {
    FreeTexture();
}

void Ball::Update(float delta_seconds) {
    velocity_y_ += gravity_ * delta_seconds;
    x_ += velocity_x_ * delta_seconds;
    y_ += velocity_y_ * delta_seconds;

    ClampAndBounce();

    // Wrap to left after passing right edge.
    if (x_ - radius_ > static_cast<float>(constants::kScreenWidth)) {
        x_ = -radius_;
        velocity_y_ = -bounce_speed_;
    }
}

void Ball::Render(SDL_Renderer* renderer) const {
    if (texture_) {
        const int scaled_width = static_cast<int>(static_cast<float>(texture_width_) * texture_scale_);
        const int scaled_height = static_cast<int>(static_cast<float>(texture_height_) * texture_scale_);
        SDL_Rect dst{
            static_cast<int>(x_ - static_cast<float>(scaled_width) * 0.5f),
            static_cast<int>(y_ - static_cast<float>(scaled_height) * 0.5f),
            scaled_width,
            scaled_height};
        SDL_RenderCopy(renderer, texture_, nullptr, &dst);
    } else {
        SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
        DrawFilledCircle(renderer, static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(radius_));
    }
}

void Ball::ClampAndBounce() {
    const float ground_y = static_cast<float>(constants::kGroundTop);
    const float max_y = ground_y - radius_;
    if (y_ >= max_y) {
        y_ = max_y;
        velocity_y_ = -bounce_speed_;
    }
}

void Ball::FreeTexture() {
    if (texture_) {
        SDL_DestroyTexture(texture_);
        texture_ = nullptr;
    }
    texture_width_ = 0;
    texture_height_ = 0;
}

void Ball::ResetToGround() {
    x_ = radius_ + 10.0f;
    y_ = constants::kGroundTop - radius_;
}
