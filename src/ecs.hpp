#include <map>
#include <vector>
#include <memory>
#include <typeindex>
#include <type_traits>
#include <iostream>

struct ECS;
struct Entity;
struct Component;
enum class SystemType;
struct System;
struct SystemData;

struct ID
{
    std::type_index first;
    unsigned int second;

    ID() : first(typeid(void)), second(static_cast<unsigned int>(-1)) {}
    ID(std::type_index first, unsigned int second) 
        : first(first), second(second) {}
    ID(std::type_index first, int second) 
        : first(first), second(static_cast<unsigned int>(second)) {}

    bool operator<(const ID& other) const 
    {
        return (first < other.first) || (first == other.first && second < other.second);
    }

    bool operator==(const ID& other) const
    {
        return first == other.first && second == other.second;
    }
};

using EntityID = ID;
using ComponentID = ID;
using SystemID = ID;
using SystemDataID = ID;

using EntityPtr = std::unique_ptr<Entity>;
using ComponentPtr = std::unique_ptr<Component>;
using SystemPtr = std::unique_ptr<System>;
using SystemDataPtr = std::unique_ptr<SystemData>;

using EntityMap = std::map<EntityID, EntityPtr>;
using ComponentMap = std::map<EntityID, std::map<ComponentID, ComponentPtr>>;
using SystemMap = std::map<SystemID, SystemPtr>;
using SystemDataMap = std::map<SystemDataID, SystemDataPtr>;

struct Entity
{
    Entity(EntityID id) : id(id) {}
    virtual ~Entity() = 0;

    virtual void onCreate(ECS* ecs) = 0;
    virtual void onDestroy(ECS* ecs) = 0;

    virtual void onAdd(ECS* ecs, ComponentID component) = 0;
    virtual void onRemove(ECS* ecs, ComponentID component) = 0;

    EntityID id;
};

struct Component
{
    Component(ComponentID id) : id(id) {}
    virtual ~Component() = 0;

    virtual void onAdd(ECS* ecs, EntityID entity) = 0;
    virtual void onRemove(ECS* ecs, EntityID entity) = 0;

    ComponentID id;
};

struct System
{
    System(SystemID id, SystemDataID data) : id(id), data(data) {}
    virtual ~System() = 0;

    virtual void onAdd(ECS* ecs) = 0;
    virtual void onRemove(ECS* ecs) = 0;

    virtual void onApply(ECS* ecs, EntityID entity) = 0;

    SystemType type;
    SystemID id;
    SystemDataID data;
};

struct SystemData
{
    SystemData(SystemDataID id) : id(id) {}
    ~SystemData();

    SystemDataID id;
};

struct ECS
{
    ECS() {}
    ~ECS() {}

    template<typename T, typename... Args>
    EntityID createEntity(Args&&... args)
    {
        std::type_index typeID = typeid(T);
        unsigned int intID = -1;
        for (unsigned int i = 0; i < MAX_ENTITIES; i++)
        {
            bool good = true;
            for (auto& j : entities)
            {
                if (j.first.first == typeID)
                {
                    if (j.first.second == i)
                    {
                        good = false;
                    }
                }
            }
            if (!good)
            {
                continue;
            } else
            {
                intID = i;
                break;
            }
        }
        
        EntityID entityID = {typeID, intID};
        if (intID == -1)
        {
            std::cout << "Failed to create entity!" << std::endl;
        } else
        {
            EntityPtr entityPtr = EntityPtr(new T(entityID, std::forward<Args>(args)...));
            entities[entityID] = std::move(entityPtr);
            entities[entityID].get()->onCreate(this);
        }
        return entityID;
    }

    bool hasEntity(EntityID entity)
    {
        auto i = entities.find(entity);
        if (i != entities.end())
        {
            return true;
        }
        return false;
    }

    template<typename T>
    T* getEntity(EntityID entity)
    {
        if (hasEntity(entity))
        {
            return reinterpret_cast<T*>(entities[entity].get());
        }
        return nullptr;
    }

    void destroyEntity(EntityID entity)
    {
        if (hasEntity(entity))
        {
            entities[entity].get()->onDestroy(this);
            entities.erase(entity);
        }
    }

    template<typename T, typename... Args>
    ComponentID addComponent(EntityID entity, Args&&... args)
    {
        std::type_index typeID = typeid(T);
        unsigned int intID = -1;
        if (hasEntity(entity))
        {
            for (unsigned int i = 0; i < MAX_COMPONENTS; i++)
            {
                bool good = true;
                auto k = components.find(entity);
                if (k != components.end())
                {
                    for (auto& j : components[entity])
                    {
                        if (j.first.first == typeID)
                        {
                            if (j.first.second == i)
                            {
                                good = false;
                            }
                        }
                    }
                    if (!good)
                    {
                        continue;
                    } else
                    {
                        intID = i;
                        break;
                    }
                } else
                {
                    intID = 0;
                    break;
                }
            }
        }
        ComponentID componentID = {typeID, intID};
        if (intID == -1)
        {
            std::cout << "Failed to add component!" << std::endl;
        } else
        {
            ComponentPtr componentPtr = ComponentPtr(new T(componentID, std::forward<Args>(args)...));
            components[entity][componentID] = std::move(componentPtr);
            components[entity][componentID].get()->onAdd(this, entity);
            entities[entity].get()->onAdd(this, componentID);
        }
        return componentID;
    }

    bool hasComponent(EntityID entity, ComponentID component)
    {
        auto i = components.find(entity);
        if (i != components.end())
        {
            auto j = i->second.find(component);
            if (j != i->second.end())
            {
                return true;
            }
        }
        return false;
    }

    template<typename T>
    std::vector<T*> getComponent(EntityID entity)
    {
        std::vector<T*> result;
        if (components.find(entity) != components.end()) {
            for (auto& [compID, compPtr] : components[entity]) {
                if (compID.first == typeid(T)) {
                    result.push_back(reinterpret_cast<T*>(compPtr.get()));
                }
            }
        }
        return result;
    }

    template<typename T>
    T* getComponent(EntityID entity, ComponentID component)
    {
        if (hasComponent(entity, component))
        {
            return reinterpret_cast<T*>(components[entity][component].get());
        }
        return nullptr;
    }

    void removeComponent(EntityID entity, ComponentID component)
    {
        if (hasComponent(entity, component))
        {
            components[entity][component].get()->onRemove(this, entity);
            entities[entity].get()->onRemove(this, component);
            components[entity].erase(component);
        }
    }

    template<typename T, typename U = SystemData, typename... Args>
    SystemID addSystem(Args&&... args)
    {
        SystemID systemID = {typeid(T), -1};
        {
            std::type_index typeID = typeid(T);
            unsigned int intID = -1;
            for (unsigned int i = 0; i < MAX_SYSTEMS; i++)
            {
                bool good = true;
                for (auto& j : systems)
                {
                    if (j.first.first == typeID)
                    {
                        if (j.first.second == i)
                        {
                            good = false;
                        }
                    }
                }
                if (!good)
                {
                    continue;
                } else
                {
                    intID = i;
                    break;
                }
            }
            systemID = {typeID, intID};
        }

        SystemDataID dataID = {typeid(U), -1};
        {
            std::type_index typeID = typeid(U);
            unsigned int intID = -1;
            for (unsigned int i = 0; i < MAX_SYSTEM_DATA; i++)
            {
                bool good = true;
                for (auto& j : systemData)
                {
                    if (j.first.first == typeID)
                    {
                        if (j.first.second == i)
                        {
                            good = false;
                        }
                    }
                }
                if (!good)
                {
                    continue;
                } else
                {
                    intID = i;
                    break;
                }
            }
            dataID = {typeID, intID};
        }

        if (dataID.second == -1)
        {
            std::cout << "Failed to add system data!" << std::endl;
        } else
        {
            SystemDataPtr dataPtr = SystemDataPtr(new U(dataID, std::forward<Args>(args)...));
            systemData[dataID] = std::move(dataPtr);
        }

        if (systemID.second == -1)
        {
            std::cout << "Failed to add system!" << std::endl;
        } else
        {
            SystemPtr systemPtr = SystemPtr(new T(systemID, dataID));
            systems[systemID] = std::move(systemPtr);
            systems[systemID].get()->onAdd(this);
        }
        
        return systemID;
    }

    bool hasSystem(SystemID system)
    {
        auto i = systems.find(system);
        if (i != systems.end())
        {
            return true;
        }
        return false;
    }

    bool hasSystemData(SystemDataID data)
    {
        auto i = systemData.find(data);
        if (i != systemData.end())
        {
            return true;
        }
        return false;
    }

    template<typename T>
    T* getSystem(SystemID system)
    {
        if (hasSystem(system))
        {
            return reinterpret_cast<T*>(systems[system].get());
        }
        return nullptr;
    }

    template<typename T>
    T* getSystemData(SystemDataID data)
    {
        if (hasSystemData(data))
        {
            return reinterpret_cast<T*>(systemData[data].get());
        }
        return nullptr;
    }

    void removeSystem(SystemID system)
    {
        if (hasSystem(system))
        {
            if (hasSystemData(systems[system].get()->data))
            {
                systemData.erase(systems[system].get()->data);
            }
            systems[system].get()->onRemove(this);
            systems.erase(system);
        }
    }

    void applySystem(SystemID system, EntityID entity)
    {
        if (hasEntity(entity) && hasSystem(system))
        {
            systems[system].get()->onApply(this, entity);
        }
    }

    void applySystems(SystemType type)
    {
        std::vector<System*> vSystems;
        for (auto& i : systems)
        {
            if (i.second.get()->type == type)
            {
                vSystems.push_back(i.second.get());
            }
        }

        for (auto j : vSystems)
        {
            for (auto& entity : entities)
            {
                j->onApply(this, entity.first);
            }
        }
    }

    const int MAX_ENTITIES = 1024;
    const int MAX_COMPONENTS = 1024;
    const int MAX_SYSTEMS = 1024;
    const int MAX_SYSTEM_DATA = 1024;

    EntityMap entities;
    ComponentMap components;
    SystemMap systems;
    SystemDataMap systemData;
};