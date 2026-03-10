#include "TemplateSC.h"
ecs::ECS* TemplateSC::ecsPtr{ nullptr };
Input::InputSystem* TemplateSC::Input{ nullptr };
scenes::SceneManager* TemplateSC::Scenes{ nullptr };
physics::PhysicsManager* TemplateSC::physicsPtr{ nullptr };
NavMeshManager* TemplateSC::navMeshPtr{ nullptr };
ResourceManager* TemplateSC::resource{nullptr};
GraphicsManager* TemplateSC::graphics{ nullptr };