// Implementation of player movement and drawing.
#include "Player.h"

#include "Constants.h"

Player::Player() {
    rect_.w = 80;
    rect_.h = 20;
    rect_.x = (constants::kScreenWidth / 2) - (rect_.w / 2);
    rect_.y = constants::kScreenHeight - 60;
}

void Player::HandleInput(const Uint8* keyboard_state) {
    const bool move_left = keyboard_state[SDL_SCANCODE_LEFT] || keyboard_state[SDL_SCANCODE_A];
    const bool move_right = keyboard_state[SDL_SCANCODE_RIGHT] || keyboard_state[SDL_SCANCODE_D];

    if (move_left == move_right) {
        velocity_x_ = 0.0f;
    } else if (move_left) {
        velocity_x_ = -speed_pixels_per_second_;
    } else if (move_right) {
        velocity_x_ = speed_pixels_per_second_;
    }
}

void Player::Update(float delta_seconds) {
    rect_.x += static_cast<int>(velocity_x_ * delta_seconds);
    ClampToScreen();
}

void Player::Render(SDL_Renderer* renderer) const {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect_);
}

void Player::ClampToScreen() {
    if (rect_.x < 0) rect_.x = 0;
    const int max_x = constants::kScreenWidth - rect_.w;
    if (rect_.x > max_x) rect_.x = max_x;
}
