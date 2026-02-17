#ifndef DEBUGMESHSYSTEM_H
#define DEBUGMESHSYSTEM_H

#include "ECS/ECS.h"
#include "System.h"

namespace ecs {
    class DebugMeshColliderRenderSystem : public ISystem {
    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;

        REFLECTABLE(DebugMeshColliderRenderSystem)
    };
}

#endif