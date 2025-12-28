# Attack On Ball Notes

- Engine: SDL2, resolution locked to 640x480 for RG35XX target.
- Entry: `src/main.cpp` boots `Game` from `src/game/Game.*`.
- Core: `Game` owns the loop (input -> update -> render) and enforces a simple frame cap.
- Player: `src/game/Player.*` handles horizontal movement/clamping, renders a 64x64 ghost sprite (`assets/characters/ghost.png`).
- Ball: `src/game/Ball.*` is a green sprite (`assets/balls/ball_green.png`) that moves left to right, bounces off the ground to a consistent height, and wraps from right edge back to left.
- Constants: `src/game/Constants.h` holds screen size, FPS, and ground dimensions/position.
- Timer: `src/game/Game.cpp` renders a centered top timer using 7-seg style digits (seconds with 0.01 precision).

## Build
- Example local build (uses pkg-config):\
  `g++ -std=c++17 src/main.cpp src/game/Game.cpp src/game/Player.cpp src/game/Ball.cpp -I./src $(pkg-config --cflags --libs sdl2 SDL2_image) -o attack_on_ball`
  - If pkg-config fails, install SDL2/SDL2_image dev headers/libs and ensure pkg-config can find them.

## Run
- `./attack_on_ball` opens a 640x480 window; move with arrow keys or A/D; close window to exit.

## Next Steps
- Add title/high score UI and start prompt before entering the loop.
- Introduce falling numbers (time pickups) and side-spawning balls with collision/death handling.
- Wire 10-second fever timing and surface HUD (timer, rank, start screen elements).
