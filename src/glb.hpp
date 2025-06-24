#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <cmath>
#include <iostream>
#include <type_traits>
#include <typeindex>
#include <random>
#include <limits>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>

#include <GLES3/gl3.h>

#include <emscripten.h>

template<typename T>
inline T randomInt(T min, T max, const std::vector<T>& exclude) {
    std::set<T> excludeSet(exclude.begin(), exclude.end());
    std::vector<T> candidates;
    for (T i = min; i <= max; ++i) {
        if (excludeSet.find(i) == excludeSet.end()) {
            candidates.push_back(i);
        }
    }
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, candidates.size() - 1);
    return candidates[dist(gen)];
}

template<typename K, typename V>
std::vector<K> getMapKeys(const std::map<K, V>& m) {
    std::vector<K> keys;
    keys.reserve(m.size());
    for (const auto& pair : m) {
        keys.push_back(pair.first);
    }
    return keys;
}

struct glb
{
    static const int WINDOW_WIDTH;
    static const int WINDOW_HEIGHT;
    static const char* WINDOW_TITLE;

    static SDL_Event* event;
    static SDL_Window** window;
    static SDL_GLContext* context;
};