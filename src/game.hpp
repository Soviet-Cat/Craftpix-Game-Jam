#pragma once

#include "glb.hpp"

class Layer
{
    public:
    virtual ~Layer() = 0;

    virtual int onInit() = 0;
    virtual void onCleanup() = 0;
    virtual void onFrame() = 0;
};

class Game
{
    public:
    friend struct glb;

    Game();
    ~Game();

    static int init();
    static void cleanup();
    static void frame();

    private:
    static SDL_Window* m_Window;
    static SDL_GLContext m_Context;
    static SDL_Event m_Event;
    static bool m_Quit;
    static std::vector<Layer> m_Layers;
};