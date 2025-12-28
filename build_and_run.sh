#!/bin/sh
set -e

g++ -std=c++17 src/main.cpp src/game/Game.cpp src/game/Player.cpp src/game/Ball.cpp -I./src $(pkg-config --cflags --libs sdl2 SDL2_image) -o attack_on_ball
./attack_on_ball
