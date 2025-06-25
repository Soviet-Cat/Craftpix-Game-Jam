#include "ecs.hpp"

Entity::~Entity() = default;
void Entity::onCreate(ECS* ecs) {}
void Entity::onDestroy(ECS* ecs) {}

Component::~Component() = default;
void Component::onAdd(ECS* ecs, EntityID entity) {}
void Component::onRemove(ECS* ecs, EntityID entity) {}

System::~System() = default;
void System::onAdd(ECS* ecs) {}
void System::onRemove(ECS* ecs) {}
void System::onApply(ECS* ecs, EntityID entity) {}

SystemData::~SystemData() = default;