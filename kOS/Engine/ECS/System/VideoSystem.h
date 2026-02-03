/******************************************************************/
/*!
\file      TransformSystem.h
\author    Jaz winn
\par       jazwinn.ng@digipen.edu
\date      Sept 29, 2024
\brief     

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/

#pragma once

#include "System.h"
#include "ECS/ECSList.h"

namespace ecs {

    class VideoSystem : public ISystem {
    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;

        REFLECTABLE(VideoSystem)

    private:
        std::unordered_map<ecs::EntityID, std::shared_ptr<R_Video>> m_videoMap;
    };

}


