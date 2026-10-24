#!/bin/bash

./scripts/download_dependencies.sh
source ./scripts/env_vars.sh
./build-scripts/ps5-payload-sdk.sh
mkdir -p build-ps5/ps5-sdl2_release/lib
cp build-ps5/libSDL2.a build-ps5/ps5-sdl2_release/lib/libSDL2.a
cp -r build-ps5/include build-ps5/ps5-sdl2_release/
cp build-ps5/SDL_config.h.intermediate build-ps5/ps5-sdl2_release/include/SDL2/SDL_config.h
