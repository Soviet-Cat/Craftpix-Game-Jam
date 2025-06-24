#pragma once

#include "glb.hpp"

class Layer
{
    public:
    Layer();
    virtual ~Layer() = 0;

    virtual int onInit() = 0;
    virtual void onCleanup() = 0;
    virtual void onTick() = 0;
    virtual void onDraw() = 0;
};

using LayerPtr = std::unique_ptr<Layer>;

class Game
{
    public:
    friend struct glb;

    Game();
    ~Game();

    static int init();
    static void cleanup();
    static void tick();
    static void draw();
    static void frame();

    template<typename T>
    static void addLayer()
    {
        m_Layers.push_back(LayerPtr(new T()));
    }

    private:
    static SDL_Window* m_Window;
    static SDL_GLContext m_Context;
    static SDL_Event m_Event;
    static bool m_Quit;
    static std::vector<LayerPtr> m_Layers;
};