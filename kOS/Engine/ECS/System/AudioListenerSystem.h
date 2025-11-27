#ifndef AUDIOLISSYS_H
#define AUDIOLISSYS_H

#include "ECS/ECS.h"
#include "System.h"
#include <glm/vec3.hpp>

namespace ecs
{
    class AudioListenerSystem : public ISystem {
    public:
        using ISystem::ISystem;

        void Init() override;
        void Update() override;

        REFLECTABLE(AudioListenerSystem)
    };

    void UpdateListenerFromComponents(ECS& ecs, glm::vec3& outPos);
}

#endif
