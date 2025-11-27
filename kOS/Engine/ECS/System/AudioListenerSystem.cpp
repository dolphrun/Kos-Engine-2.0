#include "Config/pch.h"
#include "AudioListenerSystem.h"

namespace ecs
{
    void AudioListenerSystem::Init()
    {
    }

    void AudioListenerSystem::Update()
    {
    }

    void UpdateListenerFromComponents(ECS& ecs, glm::vec3& outPos)
    {
        const auto& listenerEntities = ecs.GetComponentsEnties("AudioListenerComponent");

        for (EntityID id : listenerEntities)
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
