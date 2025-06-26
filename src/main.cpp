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

#include "../lib/glm/glm.hpp"
#include "../lib/glm/gtc/matrix_transform.hpp"
#include "../lib/glm/gtc/type_ptr.hpp"

#include "helper.hpp"
#include "ecs.hpp"

enum class SystemType
{
    TICK,
    DRAW
};

enum class SurfaceID
{
    GUI_FRAMES = 0,
    GUI_ICONS = 1,
    BASEMENT_BACKGROUND = 2
};

enum class TextureID
{
    GUI_FRAMES = 0,
    GUI_ICONS = 1,
    BASEMENT_BACKGROUND = 2
};

enum class ShaderID
{
    DEFAULT,
    SAMPLER,
    PARALLAX
};

enum class MeshID
{
    QUAD_32x32,
    QUAD_640x360
};

enum class FontID
{
    CYBERPUNK_CRAFTPIX_PIXEL
};

enum class LevelType
{
    MAINMENU,
    SEWERAGE
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

    const int PIXEL_WIDTH = 640;
    const int PIXEL_HEIGHT = 360;
    const glm::vec2 RENDER_SCALE = glm::vec2(2.0 / PIXEL_WIDTH, 2.0 / PIXEL_HEIGHT);
    glm::vec2 renderOffset;

    std::map<SDL_Keycode, bool> keyboard;

    ECS ecs;

    EntityID surfacePlaceHolder;
    EntityID texturePlaceHolder;
    EntityID shaderPlaceHolder;
    EntityID meshPlaceHolder;
    EntityID fontPlaceHolder;

    EntityID level;
} glb;

template<typename T = int>
struct PlaceHolderEntity : public Entity
{
    PlaceHolderEntity(EntityID id) : Entity(id) {}
    ~PlaceHolderEntity() override = default;

    void onCreate(ECS* ecs) override {}
    void onDestroy(ECS* ecs) override 
    {
        for (auto component : components)
        {
            ecs->removeComponent(id, component);
        }
    }

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

    ComponentID& operator[](T i)
    {
        return components[static_cast<size_t>(i)];
    }

    const ComponentID& operator[](T i) const
    {
        return components[static_cast<size_t>(i)];
    }

    std::vector<ComponentID> components;
};

template<typename T, typename U>
T* getPlaceHolderComponent(EntityID placeHolder, const U& index)
{
    auto* entity = glb.ecs.getEntity<PlaceHolderEntity<U>>(placeHolder);
    auto* component = glb.ecs.getComponent<T>(placeHolder, (*entity)[index]);
    return component;
};

struct SurfaceComponent : public Component
{
    SurfaceComponent(
        ComponentID id,
        const std::string& path
    ) : Component(id), path(path) {}
    ~SurfaceComponent() override = default;

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        surface = IMG_Load(path.c_str());
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        SDL_FreeSurface(surface);
    }

    std::string path;
    SDL_Surface* surface;
};

struct TextureComponent : public Component
{
    TextureComponent(ComponentID id, SurfaceID surface) : Component(id), surface(surface) {}
    ~TextureComponent() override = default;

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        auto* component = getPlaceHolderComponent<SurfaceComponent, SurfaceID>(glb.surfacePlaceHolder, surface);

        SDL_Surface* formatted = SDL_ConvertSurfaceFormat(component->surface, SDL_PIXELFORMAT_RGBA32, 0);

        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, formatted->w, formatted->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, formatted->pixels);

        width = formatted->w;
        height = formatted->h;

        SDL_FreeSurface(formatted);
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        glDeleteTextures(1, &id);
    }

    SurfaceID surface;
    GLuint id;
    int width;
    int height;
};

struct ShaderComponent : public Component
{
    ShaderComponent(
        ComponentID id,
        const std::string& vertexSrc,
        const std::string& fragmentSrc
    ) : Component(id), vertexSrc(vertexSrc), fragmentSrc(fragmentSrc) {}
    ~ShaderComponent() override = default;

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        vertex = glCreateShader(GL_VERTEX_SHADER);
        fragment = glCreateShader(GL_FRAGMENT_SHADER);

        const char* pVertexSrc = vertexSrc.c_str();
        glShaderSource(vertex, 1, &pVertexSrc, nullptr);
        const char* pFragmentSrc = fragmentSrc.c_str();
        glShaderSource(fragment, 1, &pFragmentSrc, nullptr);

        glCompileShader(vertex);
        glCompileShader(fragment);

        GLint vertexResult;
        glGetShaderiv(vertex, GL_COMPILE_STATUS, &vertexResult);

        GLint fragmentResult;
        glGetShaderiv(fragment, GL_COMPILE_STATUS, &fragmentResult);

        if (vertexResult == GL_FALSE)
        {
            GLint length;
            glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);
            char* msg = new char[length];
            glGetShaderInfoLog(vertex, length, &length, msg);
            std::cout << "Failed to compile vertex shader!" << std::endl;
            std::cout << msg << std::endl;
            delete[] msg;
        }

        if (fragmentResult == GL_FALSE)
        {
            GLint length;
            glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);
            char* msg = new char[length];
            glGetShaderInfoLog(fragment, length, &length, msg);
            std::cout << "Failed to compile fragment shader!" << std::endl;
            std::cout << msg << std::endl;
            delete[] msg;
        }

        program = glCreateProgram();

        glAttachShader(program, vertex);
        glAttachShader(program, fragment);

        glLinkProgram(program);

        GLint linkResult;
        glGetProgramiv(program, GL_LINK_STATUS, &linkResult);

        if (linkResult == GL_FALSE)
        {
            GLint length;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
            char* msg = new char[length];
            glGetProgramInfoLog(program, length, &length, msg);
            std::cout << "Failed to link shader program!" << std::endl;
            std::cout << msg << std::endl;
            delete[] msg;
        }
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        glDeleteProgram(program);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    std::string vertexSrc;
    std::string fragmentSrc;
    GLuint vertex;
    GLuint fragment;
    GLuint program;
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
    MeshComponent(
        ComponentID id,
        MeshType type,
        const glm::vec2& size
    ) : Component(id), type(type)
    {
        glm::vec2 scaledSize = size * glb.RENDER_SCALE;
        glm::vec2 halfSize = scaledSize / 2.0f;
        vertices = {
            -halfSize.x, halfSize.y, 0.0f, 1.0f,
            -halfSize.x, -halfSize.y, 0.0f, 0.0f,
            halfSize.x, -halfSize.y, 1.0f, 0.0f,
            halfSize.x, halfSize.y, 1.0f, 1.0f
        };
        elements = {
            0, 1, 2, 2, 3, 0
        };
        attributes = {
            {0, 2}, {1, 2}
        };
    }
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

        GLsizei stride;
        GLint size = 0;

        for (auto& attribute : attributes)
        {
            size += attribute.second;
        }

        stride = size * sizeof(GLfloat);

        for (size_t i = 0; i < attributes.size(); i++)
        {
            auto& attribute = attributes[i];
            const void* offset = (const void*)(0);

            if (i > 0)
            {
                auto& lastAttribute = attributes[i - 1];
                offset = (const void*)(lastAttribute.second * sizeof(GLfloat));
            }

            glEnableVertexAttribArray(attribute.first);
            glVertexAttribPointer(
                attribute.first,
                attribute.second,
                GL_FLOAT,
                GL_FALSE,
                stride,
                offset
            );
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

struct FontComponent : public Component
{
    FontComponent(ComponentID id, const std::string& path, int size) 
        : Component(id), path(path), size(size) {}
    ~FontComponent() = default;

    void onAdd(ECS* ecs, EntityID entity) override 
    {
        font = TTF_OpenFont(path.c_str(), size);
    }
    void onRemove(ECS* ecs, EntityID entity) override 
    {
        TTF_CloseFont(font);
    }

    TTF_Font* font;
    std::string path;
    int size;
};

struct TransformComponent : public Component
{
    TransformComponent(ComponentID id, const glm::vec2& position, float rotation, const glm::vec2& scale)
        : Component(id), position(position), rotation(rotation), scale(scale) {}
    ~TransformComponent() = default;

    void onAdd(ECS* ecs, EntityID entity) override {}
    void onRemove(ECS* ecs, EntityID entity) override {}

    glm::vec2 position;
    float rotation;
    glm::vec2 scale;
};

struct TextureSamplerComponent : public Component
{
    TextureSamplerComponent(ComponentID id, ComponentID transform, TextureID texture, MeshID mesh, const glm::vec2& size, const glm::vec2& portion, bool flipX, bool flipY, bool ui) 
        : Component(id), transform(transform), texture(texture), mesh(mesh), size(size), portion(portion), flipX(flipX), flipY(flipY), ui(ui) {}
    ~TextureSamplerComponent() = default;

    void onAdd(ECS* ecs, EntityID entity) override {}
    void onRemove(ECS* ecs, EntityID entity) override {}

    ComponentID transform;
    TextureID texture;
    MeshID mesh;
    glm::vec2 size;
    glm::vec2 portion;
    bool flipX;
    bool flipY;
    bool ui;
};

struct TextureSamplerSystemData : public SystemData
{
    TextureSamplerSystemData(SystemDataID id)
        : SystemData(id) {}
    ~TextureSamplerSystemData() = default;

    ShaderComponent* shader;
    GLint uModel;
    GLint uSampler2D;
    GLint uSize;
    GLint uPortion;
    GLint uFlipX;
    GLint uFlipY;
};

struct TextureSamplerSystem : public System
{
    TextureSamplerSystem(SystemID id, SystemDataID data)
        : System(id, data) 
        {
            type = SystemType::DRAW;
        }
    ~TextureSamplerSystem() = default;

    void onAdd(ECS* ecs) override 
    {
        auto* pData = ecs->getSystemData<TextureSamplerSystemData>(data);
        pData->shader = getPlaceHolderComponent<ShaderComponent, ShaderID>(glb.shaderPlaceHolder, ShaderID::SAMPLER);
        pData->uModel = glGetUniformLocation(pData->shader->program, "model");
        pData->uSampler2D = glGetUniformLocation(pData->shader->program, "tex");
        pData->uSize = glGetUniformLocation(pData->shader->program, "size");
        pData->uPortion = glGetUniformLocation(pData->shader->program, "portion");
        pData->uFlipX = glGetUniformLocation(pData->shader->program, "flipX");
        pData->uFlipY = glGetUniformLocation(pData->shader->program, "flipY");
    }
    void onRemove(ECS* ecs) override {}

    void onApply(ECS* ecs, EntityID entity) override 
    {            
        auto* pData = ecs->getSystemData<TextureSamplerSystemData>(data);

        glUseProgram(pData->shader->program);

        auto samplers = ecs->getComponent<TextureSamplerComponent>(entity);
        for (auto* sampler : samplers)
        {
            auto* transform = ecs->getComponent<TransformComponent>(entity, sampler->transform);
            auto* texture = getPlaceHolderComponent<TextureComponent, TextureID>(glb.texturePlaceHolder, sampler->texture);
            auto* mesh = getPlaceHolderComponent<MeshComponent, MeshID>(glb.meshPlaceHolder, sampler->mesh);

            GLint textureSlot = static_cast<GLint>(sampler->texture);
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(GL_TEXTURE_2D, texture->id);

            glm::vec2 position = (transform->position + glb.renderOffset) * glb.RENDER_SCALE;
            if (sampler->ui)
            {
                position = transform->position * glb.RENDER_SCALE;
            }

            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
            model = glm::rotate(model, glm::radians(transform->rotation), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(transform->scale, 1.0f));

            glUniformMatrix4fv(pData->uModel, 1, GL_FALSE, &model[0][0]);

            glm::vec2 scale = glm::vec2(1.0 / texture->width, 1.0 / texture->height);

            glUniform1i(pData->uSampler2D, textureSlot);
            glUniform2f(pData->uSize, sampler->size.x * scale.x, sampler->size.y * scale.y);
            glUniform2f(pData->uPortion, sampler->portion.x * scale.x, sampler->portion.y * scale.y);
            glUniform1i(pData->uFlipX, sampler->flipX ? 1 : 0);
            glUniform1i(pData->uFlipY, sampler->flipY ? 1 : 0);

            glBindVertexArray(mesh->vao);

            glDrawElements(GL_TRIANGLES, mesh->elements.size(), GL_UNSIGNED_INT, 0);
            
            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
};

struct ParallaxBackgroundComponent : public Component
{
    ParallaxBackgroundComponent(ComponentID id, TextureID texture, int layers)
        : Component(id), texture(texture), layers(layers) {}
    ~ParallaxBackgroundComponent() = default;

    void onAdd(ECS* ecs, EntityID entity) override {}
    void onRemove(ECS* ecs, EntityID entity) override {}

    TextureID texture;
    int layers;
};

struct ParallaxBackgroundSystemData : public SystemData
{
    ParallaxBackgroundSystemData(SystemDataID id) : SystemData(id) {}
    ~ParallaxBackgroundSystemData() = default;

    ShaderComponent* shader;
    GLint uSampler2D;
    GLint uOffset;
    GLint uLayer;
    GLint uMaxLayer;
};

struct ParallaxBackgroundSystem : public System
{
    ParallaxBackgroundSystem(SystemID id, SystemDataID data) : System(id, data) 
    {
        type = SystemType::DRAW;
    }
    ~ParallaxBackgroundSystem() = default;

    void onAdd(ECS* ecs) override 
    {
        auto* pData = ecs->getSystemData<ParallaxBackgroundSystemData>(data);
        pData->shader = getPlaceHolderComponent<ShaderComponent, ShaderID>(glb.shaderPlaceHolder, ShaderID::PARALLAX);
        pData->uSampler2D = glGetUniformLocation(pData->shader->program, "tex");
        pData->uOffset = glGetUniformLocation(pData->shader->program, "offset");
        pData->uLayer = glGetUniformLocation(pData->shader->program, "layer");
        pData->uMaxLayer = glGetUniformLocation(pData->shader->program, "maxLayer");
    }
    void onRemove(ECS* ecs) override {}

    void onApply(ECS* ecs, EntityID entity) override 
    {
        auto* pData = ecs->getSystemData<ParallaxBackgroundSystemData>(data);

        glUseProgram(pData->shader->program);

        auto backgrounds = ecs->getComponent<ParallaxBackgroundComponent>(entity);
        for (auto* background : backgrounds)
        {
            auto* texture = getPlaceHolderComponent<TextureComponent, TextureID>(glb.texturePlaceHolder, background->texture);
            auto* mesh = getPlaceHolderComponent<MeshComponent, MeshID>(glb.meshPlaceHolder, MeshID::QUAD_640x360);

            GLint textureSlot = static_cast<GLint>(background->texture);
            glActiveTexture(GL_TEXTURE0 + textureSlot);
            glBindTexture(GL_TEXTURE_2D, texture->id);

            glBindVertexArray(mesh->vao);

            for (int i = 0; i < background->layers; i++)
            {
                glUniform1i(pData->uSampler2D, textureSlot);
                glUniform2f(pData->uOffset, glb.renderOffset.x * glb.RENDER_SCALE.x, glb.renderOffset.y * glb.RENDER_SCALE.y);
                glUniform1f(pData->uLayer, static_cast<GLfloat>(i));
                glUniform1f(pData->uMaxLayer, static_cast<GLfloat>(background->layers));

                glDrawElements(GL_TRIANGLES, mesh->elements.size(), GL_UNSIGNED_INT, 0);
            }

            glBindVertexArray(0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }                         
};

struct MainMenuLevelEntity : public Entity
{
    MainMenuLevelEntity(EntityID id)
        : Entity(id) {}
    ~MainMenuLevelEntity() = default;

    void onCreate(ECS* ecs) override
    {
        ecs->addComponent<ParallaxBackgroundComponent>(id,
            TextureID::BASEMENT_BACKGROUND,
            5
        );
        ecs->addSystem<ParallaxBackgroundSystem, ParallaxBackgroundSystemData>();
    }
    void onDestroy(ECS* ecs) override
    {
        
    }

    void onAdd(ECS* ecs, ComponentID component) override {}
    void onRemove(ECS* ecs, ComponentID component) override {}
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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glb.surfacePlaceHolder = glb.ecs.createEntity<PlaceHolderEntity<SurfaceID>>();
    glb.ecs.addComponent<SurfaceComponent>(glb.surfacePlaceHolder, "assets/gui_frames.png");
    glb.ecs.addComponent<SurfaceComponent>(glb.surfacePlaceHolder, "assets/gui_icons.png");
    glb.ecs.addComponent<SurfaceComponent>(glb.surfacePlaceHolder, "assets/basement_background.png");

    glb.texturePlaceHolder = glb.ecs.createEntity<PlaceHolderEntity<TextureID>>();
    glb.ecs.addComponent<TextureComponent>(glb.texturePlaceHolder, SurfaceID::GUI_FRAMES);
    glb.ecs.addComponent<TextureComponent>(glb.texturePlaceHolder, SurfaceID::GUI_ICONS);
    glb.ecs.addComponent<TextureComponent>(glb.texturePlaceHolder, SurfaceID::BASEMENT_BACKGROUND);

    glb.shaderPlaceHolder = glb.ecs.createEntity<PlaceHolderEntity<ShaderID>>();
    glb.ecs.addComponent<ShaderComponent>(glb.shaderPlaceHolder, readFile("shaders/default_vertex.glsl"), readFile("shaders/default_fragment.glsl"));
    glb.ecs.addComponent<ShaderComponent>(glb.shaderPlaceHolder, readFile("shaders/sampler_vertex.glsl"), readFile("shaders/sampler_fragment.glsl"));
    glb.ecs.addComponent<ShaderComponent>(glb.shaderPlaceHolder, readFile("shaders/parallax_vertex.glsl"), readFile("shaders/parallax_fragment.glsl"));

    glb.meshPlaceHolder = glb.ecs.createEntity<PlaceHolderEntity<MeshID>>();
    glb.ecs.addComponent<MeshComponent>(glb.meshPlaceHolder, MeshType::STATIC, glm::vec2{32.0f, 32.0f});
    glb.ecs.addComponent<MeshComponent>(glb.meshPlaceHolder, MeshType::STATIC, glm::vec2{640.0f, 360.0f});

    glb.fontPlaceHolder = glb.ecs.createEntity<PlaceHolderEntity<FontID>>();
    glb.ecs.addComponent<FontComponent>(glb.fontPlaceHolder, "assets/CyberpunkCraftpixPixel.otf", 32);

    glb.level = glb.ecs.createEntity<MainMenuLevelEntity>();
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