# Attack On Ball Notes

- Engine: SDL2, resolution locked to 640x480 for RG35XX target.
- Entry: `src/main.cpp` boots `Game` from `src/game/Game.*`.
- Core: `Game` owns the loop (input -> update -> render) and enforces a simple frame cap; `Player` lives in `src/game/Player.*` and handles horizontal movement/clamping.
- Constants: `src/game/Constants.h` holds screen size and FPS.

## Build
- Example local build (uses pkg-config):\
  `g++ -std=c++17 src/main.cpp src/game/Game.cpp src/game/Player.cpp -I./src $(pkg-config --cflags --libs sdl2) -o attack_on_ball`
  - If pkg-config fails, install SDL2 dev headers/libs and ensure pkg-config can find them.

## Run
- `./attack_on_ball` opens a 640x480 window; move with arrow keys or A/D; close window to exit.

## Next Steps
- Add title/high score UI and start prompt before entering the loop.
- Introduce falling numbers (time pickups) and side-spawning balls with collision/death handling.
- Wire 10-second fever timing and surface HUD (timer, rank, start screen elements).
