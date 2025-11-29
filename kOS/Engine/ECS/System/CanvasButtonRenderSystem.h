#ifndef CANVASBUTTONRENDERSYSTEM_H
#define CANVASBUTTONRENDERSYSTEM_H

#include "ECS/ECS.h"
#include "System.h"

namespace ecs {
    class CanvasButtonRenderSystem : public ISystem {
    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;

        REFLECTABLE(CanvasButtonRenderSystem)
    };
}

#endif 