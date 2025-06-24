#include "glb.hpp"

using EntityType = std::type_index;
using ComponentType = std::type_index;
using SystemType = std::type_index;
using EntityID = unsigned int;
using ComponentID = unsigned int;
using SystemID = unsigned int;

class Entity
{
    public:
    Entity(EntityID id);
    virtual ~Entity() = 0;

    virtual void onCreate() = 0;
    virtual void onDestroy() = 0;

    private:
    EntityID id;
};

class Component
{
    public:
    Component(ComponentID id);
    virtual ~Component() = 0;

    virtual void onAdd() = 0;
    virtual void onRemove() = 0;

    private:
    ComponentID id;
};

class System
{
    public:
    System(SystemID id);
    virtual ~System() = 0;

    virtual void onAdd() = 0;
    virtual void onRemove() = 0;
    virtual void onTick() = 0;
    virtual void onDraw() = 0;

    private:
    SystemID id;
};

using EntityPtr = std::unique_ptr<Entity>;
using ComponentPtr = std::unique_ptr<Component>;
using SystemPtr = std::unique_ptr<System>;

using EntityMap = std::map<EntityType, std::map<EntityID, EntityPtr>>;
using ComponentMap = std::map<EntityID, std::map<ComponentType, std::map<ComponentID, ComponentPtr>>>;
using SystemMap = std::map<SystemType, std::map<SystemID, SystemPtr>>;

class ECS
{
    public:
    ECS();
    ~ECS();

    template<typename T>
    EntityID createEntity()
    {
        EntityType typeID = typeid(T);
        EntityID entityID = randomInt<EntityID>(std::numeric_limits<EntityID>::min(), std::numeric_limits<EntityID>::max(), getMapKeys<EntityID, EntityPtr>(m_Entities));
        m_Entities[typeID][entityID] = EntityPtr(new T(entityID));
        return entityID;
    }

    template<typename T>
    bool hasEntity(EntityID entity)
    {
        EntityType typeID = typeid(T);
        auto i = m_Entities.find(typeID);
        if (i != m_Entities.end())
        {
            auto j = i->second.find(entity);
            if (j != i->second.end())
            {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    T* getEntity(EntityID entity)
    {
        if (hasEntity<T>(entity))
        {
            return dynamic_cast<T*>(m_Entities[typeid(T)][entity].get());
        }
        return nullptr;
    }

    template<typename T>
    void destroyEntity(EntityID entity)
    {
        if (hasEntity<T>(entity))
        {
            m_Entities[typeid(T)].erase(entity);
            auto i = m_Components.find(entity);
            if (i != m_Components.end())
            {
                m_Components.erase(entity);   
            }
        }
    }

    template<typename T, typename... Args>
    ComponentID addComponent(EntityID entity, Args&&... args)
    {
        ComponentType typeID = typeid(T);
        std::vector<ComponentID> exclude;
        auto i = m_Components.find(entity);
        if (i != m_Components.end())
        {
            auto j = i->second.find(typeID);
            if (j != i->second.end())
            {
                exclude = getMapKeys<ComponentID, ComponentPtr>(j->second);
            } else
            {
                exclude = {};
            }
        } else
        {
            exclude = {};
        }
        ComponentID componentID = randomInt<ComponentID>(std::numeric_limits<ComponentID>::min(), std::numeric_limits<ComponentID>::max(), exclude);
        m_Components[typeID][componentID] = ComponentPtr(new T(std::forward<Args>(args)...));
        return componentID;
    }

    template<typename T>
    std::vector<T*> getComponent(EntityID entity)
    {

    }

    template<typename T>
    T* getComponent(EntityID entity, ComponentID component)
    {

    }

    template<typename T>
    void removeComponent(EntityID entity, ComponentID component)
    {

    }

    template<typename T>
    SystemID addSystem()
    {

    }

    template<typename T>
    void removeSystem(SystemID system)
    {

    }

    private:
    EntityMap m_Entities;
    ComponentMap m_Components;
    SystemMap m_Systems;
};