#ifndef BUTTONSYS_H
#define BUTTONSYS_H

#include "System.h"
#include "ECS/ECSList.h"
#include "Events/Delegate.h"

namespace ecs {

    class ButtonSystem : public ISystem {

    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;

        REFLECTABLE(ButtonSystem)
    private:

    };

}

#endif