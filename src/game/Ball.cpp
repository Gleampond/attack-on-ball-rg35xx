// Implementation of a simple bouncing ball.
#include "Ball.h"

#include <SDL2/SDL.h>
#include <cmath>

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
    x_ = radius_ + 10.0f;
    y_ = constants::kGroundTop - radius_;
    velocity_y_ = -bounce_speed_;
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
    SDL_SetRenderDrawColor(renderer, 200, 40, 40, 255);
    DrawFilledCircle(renderer, static_cast<int>(x_), static_cast<int>(y_), static_cast<int>(radius_));
}

void Ball::ClampAndBounce() {
    const float ground_y = static_cast<float>(constants::kGroundTop);
    const float max_y = ground_y - radius_;
    if (y_ >= max_y) {
        y_ = max_y;
        velocity_y_ = -bounce_speed_;
    }
}
