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

enum class SystemType
{
    TICK,
    DRAW
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

    EntityID shaderPlaceHolder;
    EntityID meshPlaceHolder;
} glb;

struct PlaceHolderEntity : public Entity
{
    PlaceHolderEntity(EntityID id) : Entity(id) {}
    ~PlaceHolderEntity() override = default;

    void onCreate(ECS* ecs) override {}
    void onDestroy(ECS* ecs) override {}

    void onAdd(ECS* ecs, ComponentID component) override 
    {
        components.push_back(component);
    }

    void onRemove(ECS* ecs, ComponentID component) override
    {
        auto i = std::find(components.begin(), components.end(), component);
        components.erase(i);
    }

    ComponentID& operator[](size_t i)
    {
        return components[i];
    }

    const ComponentID& operator[](size_t i) const
    {
        return components[i];
    }

    std::vector<ComponentID> components;
};

struct ShaderComponent : public Component
{

};

enum class MeshType
{    
    STATIC
};

struct MeshComponent : public Component
{
    MeshComponent(
        ComponentID id,
        MeshType type, 
        std::vector<GLfloat> vertices,
        std::vector<GLuint> elements,
        std::vector<std::pair<GLuint, GLint>> attributes
    ) : Component(id),
        type(type), vertices(vertices), elements(elements), attributes(attributes) {}
    ~MeshComponent() override = default;

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        switch (type)
        {
            case MeshType::STATIC:
            {
                glGenBuffers(1, &vbo);
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

                glGenBuffers(1, &ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, elements.size() * sizeof(GLuint), elements.data(), GL_STATIC_DRAW);
            }
        }

        glBindVertexArray(0);
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }

    MeshType type;
    std::vector<GLfloat> vertices;
    std::vector<GLuint> elements;
    std::vector<std::pair<GLuint, GLint>> attributes;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;    
};

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

    glb.shaderPlaceHolder = glb.ecs.createEntity<PlaceHolderEntity>();
    glb.meshPlaceHolder = glb.ecs.createEntity<PlaceHolderEntity>();
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
            case SDL_KEYDOWN:
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

    glb.ecs.applySystems(SystemType::TICK);

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glb.ecs.applySystems(SystemType::DRAW);

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