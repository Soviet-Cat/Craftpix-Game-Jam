#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <string>
#include <cmath>
#include <iostream>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <GLES3/gl3.h>

#include <emscripten.h>

struct glb
{
    static const int WINDOW_WIDTH;
    static const int WINDOW_HEIGHT;
    static const char* WINDOW_TITLE;

    static SDL_Event* event;
    static SDL_Window** window;
    static SDL_GLContext* context;
};