#include "Config/pch.h"
#include "AudioListenerSystem.h"
#include "Resources/ResourceManager.h"

namespace ecs
{
    void AudioListenerSystem::Init()
    {
    }

    void AudioListenerSystem::Update()
    {
    }

    void AudioListenerSystem::UpdateListenerFromComponents(ECS& ecs, glm::vec3& outPos)
    {
        //const auto& listenerEntities = ecs.GetComponentsEnties("AudioListenerComponent");
        const auto& entities = m_entities.Data();

        for (EntityID id : entities)
        {
            auto* listener = ecs.GetComponent<AudioListenerComponent>(id);
            auto* transform = ecs.GetComponent<TransformComponent>(id);

            if (!listener || !transform) continue;
            if (!listener->active)       continue;

            outPos = transform->WorldTransformation.position;
            break;
        }
    }
}
