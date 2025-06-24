#include "game.hpp"

Layer::Layer() = default;
Layer::~Layer() = default;

int Layer::onInit() {return 0;};
void Layer::onCleanup() {};
void Layer::onTick() {};
void Layer::onDraw() {};

Game::Game() = default;
Game::~Game() = default;

SDL_Window* Game::m_Window = nullptr;
SDL_GLContext Game::m_Context = {};
SDL_Event Game::m_Event = {};
bool Game::m_Quit = false;
std::vector<LayerPtr> Game::m_Layers = {};

int Game::init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK) != 0)
    {
        std::cout << "SDL_Init failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0)
    {
        std::cout << "IMG_Init failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }
    if (TTF_Init() != 0)
    {
        std::cout << "TTF_Init failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }
    if (Mix_Init(MIX_INIT_OGG) == 0)
    {
        std::cout << "MIX_Init failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }

    glb::event = &m_Event;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    m_Window = SDL_CreateWindow(
        glb::WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        glb::WINDOW_WIDTH,
        glb::WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );
    glb::window = &m_Window;
    if (!m_Window) 
    {
        std::cout << "SDL_CreateWindow failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }

    m_Context = SDL_GL_CreateContext(m_Window);
    if (!m_Context) 
    {
        std::cout << "SDL_GL_CreateContext failed!" << std::endl;
        std::cout << SDL_GetError() << std::endl;
        return -1;
    }
    glb::context = &m_Context;

    glViewport(0, 0, glb::WINDOW_WIDTH, glb::WINDOW_HEIGHT);

    int result = 0;
    for (auto& layer : m_Layers)
    {
        Layer* ptr = layer.get();
        if (ptr)
        {
            int r = ptr->onInit();
            if (r != 0)
            {
                result = r;
            }
        }
    }

    return result;
}

void Game::cleanup()
{
    for (auto& layer : m_Layers)
    {
        Layer* ptr = layer.get();
        if (ptr)
        {
            ptr->onCleanup();
        }
    }

    SDL_GL_DeleteContext(m_Context);
    SDL_DestroyWindow(m_Window);
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void Game::tick()
{
    for (auto& layer : m_Layers)
    {
        Layer* ptr = layer.get();
        if (ptr)
        {
            ptr->onTick();
        }
    }
}

void Game::draw()
{
    for (auto& layer : m_Layers)
    {
        Layer* ptr = layer.get();
        if (ptr)
        {
            ptr->onDraw();
        }
    }
}

void Game::frame()
{
    while (SDL_PollEvent(&m_Event))
    {
        switch (m_Event.type)
        {
            case SDL_QUIT:
            {
                m_Quit = true;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    tick();

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    draw();

    SDL_GL_SwapWindow(m_Window);

    if (m_Quit)
    {
        emscripten_cancel_main_loop();
        Game::cleanup();
    }
}