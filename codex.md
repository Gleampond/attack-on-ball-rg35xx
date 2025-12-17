# Attack On Ball Notes

- Engine: SDL2, resolution locked to 640x480 for RG35XX target.
- Entry: `src/main.cpp` boots `Game` from `src/game/Game.*`.
- Core: `Game` owns the loop (input -> update -> render) and enforces a simple frame cap.
- Player: `src/game/Player.*` handles horizontal movement/clamping, renders a 64x64 ghost sprite (`assets/characters/ghost-v1.bmp`) with a pink outline.
- Ball: `src/game/Ball.*` is a red circle that moves left to right, bounces off the ground to a consistent height, and wraps from right edge back to left.
- Constants: `src/game/Constants.h` holds screen size, FPS, and ground dimensions/position.

## Build
- Example local build (uses pkg-config):\
  `g++ -std=c++17 src/main.cpp src/game/Game.cpp src/game/Player.cpp src/game/Ball.cpp -I./src $(pkg-config --cflags --libs sdl2) -o attack_on_ball`
  - If pkg-config fails, install SDL2 dev headers/libs and ensure pkg-config can find them.

## Run
- `./attack_on_ball` opens a 640x480 window; move with arrow keys or A/D; close window to exit.

## Next Steps
- Add title/high score UI and start prompt before entering the loop.
- Introduce falling numbers (time pickups) and side-spawning balls with collision/death handling.
- Wire 10-second fever timing and surface HUD (timer, rank, start screen elements).
