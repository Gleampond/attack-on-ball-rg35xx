#!/bin/sh
set -e

g++ -std=c++17 \
  src/main.cpp \
  src/game/Game.cpp \
  src/game/Assets.cpp \
  src/game/BitmapFont.cpp \
  src/game/ScoreStorage.cpp \
  src/game/states/BootState.cpp \
  src/game/states/PreloadState.cpp \
  src/game/states/MenuState.cpp \
  src/game/states/GameState.cpp \
  src/game/states/ResultState.cpp \
  -I./src \
  $(pkg-config --cflags --libs sdl2 SDL2_image SDL2_mixer) \
  -o attack_on_ball
./attack_on_ball
