#include <string>
#include <iostream>
#include <memory>
#include <vector>
#include <map>

#include <emscripten.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <GLES3/gl3.h>

#include "ecs.hpp"

struct TestEntity : public Entity
{
    TestEntity(EntityID id) : Entity(id) {}
    virtual ~TestEntity() {}

    void onCreate(ECS* ecs) override 
    {
        std::cout << "Created test entity! at id: {" << id.first.name() << ", " << id.second << "}" << std::endl;
    }
    void onDestroy(ECS* ecs) override 
    {
        std::cout << "Destroyed test entity! at id: {" << id.first.name() << ", " << id.second << "}" << std::endl;
    }
};

struct TestComponent : public Component
{
    TestComponent(ComponentID id) : Component(id) {}
    virtual ~TestComponent() {}

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        std::cout << "Added test component! to entity: {" << entity.first.name() << ", " << entity.second << "} at index: {" << id.first.name() << ", " << id.second << "}" << std::endl;
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        std::cout << "Removed test component! to entity: {" << entity.first.name() << ", " << entity.second << "} at index: {" << id.first.name() << ", " << id.second << "}" << std::endl;
    }
};

struct glb_t
{
    const int WINDOW_WIDTH = 1280;
    const int WINDOW_HEIGHT = 720;
    const char* WINDOW_TITLE = "CyberSwitch";

    SDL_Window* window;
    SDL_GLContext context;
    SDL_Event event;
    bool quit = false;

    ECS ecs;
} glb;

void init()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init();
    Mix_Init(MIX_INIT_OGG);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    glb.window = SDL_CreateWindow(
        glb.WINDOW_TITLE,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        glb.WINDOW_WIDTH, glb.WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    glb.context = SDL_GL_CreateContext(glb.window);
    glViewport(0, 0, glb.WINDOW_WIDTH, glb.WINDOW_HEIGHT);

    EntityID e = glb.ecs.createEntity<TestEntity>();
    std::cout << glb.ecs.hasEntity(e) << std::endl;
    TestEntity* ep = glb.ecs.getEntity<TestEntity>(e);
    std::cout << ep << std::endl;
    ComponentID c = glb.ecs.addComponent<TestComponent>(e);
    std::cout << glb.ecs.hasComponent(e, c) << std::endl;
    TestComponent* cp = glb.ecs.getComponent<TestComponent>(e, c);
    std::cout << cp << std::endl;
    glb.ecs.removeComponent(e, c);
    std::cout << glb.ecs.hasComponent(e, c) << std::endl;
    cp = glb.ecs.getComponent<TestComponent>(e, c);
    std::cout << (cp == nullptr) << std::endl;
    glb.ecs.destroyEntity(e);
    std::cout << glb.ecs.hasEntity(e) << std::endl;
    ep = glb.ecs.getEntity<TestEntity>(e);
    std::cout << (ep == nullptr) << std::endl;
}

void cleanup()
{
    SDL_GL_DeleteContext(glb.context);
    SDL_DestroyWindow(glb.window);

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

void loop()
{
    while (SDL_PollEvent(&glb.event))
    {
        switch (glb.event.type)
        {
            case SDL_QUIT:
            {
                glb.quit = true;
                break;
            }
            default:
            {
                break;
            }
        }
    }

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(glb.window);

    if (glb.quit)
    {
        emscripten_cancel_main_loop();
        cleanup();
    }
}

int main()
{
    init();

    emscripten_set_main_loop(loop, 0, 1);
    emscripten_set_main_loop_timing(EM_TIMING_RAF, 1);
}