/********************************************************************/
/*!
\file      DebugBoxColliderRenderSystem.cpp
\author    Sean Tiu (2303398)
\par       Email: s.tiu@digipen.edu
\date      Oct 01, 2025
\brief     Defines the DebugBoxColliderRenderSystem class, which
           visualizes box colliders for debugging purposes within
           the ECS system.

           This system:
           - Draws wireframe representations of box colliders.
           - Aids in debugging physics and collision components.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "ECS/ECS.h"

#include "DebugBoxColliderRenderSystem.h"
#include "Resources/ResourceManager.h"

#include "Graphics/GraphicsManager.h"

namespace ecs {

    void DebugBoxColliderRenderSystem::Init()
    {
        // Initialize static mesh rendering resources if needed
    }

    void DebugBoxColliderRenderSystem::Update()
    {

        const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            BoxColliderComponent* box = m_ecs.GetComponent<BoxColliderComponent>(id);

            if (!transform || !box) { continue; }

            glm::vec3 scale = transform->WorldTransformation.scale;
            glm::vec3 size = box->box.size * scale;
            glm::quat rot = glm::quat(glm::radians(transform->WorldTransformation.rotation));

            glm::vec3 center = transform->WorldTransformation.position + rot * (box->box.center * scale);

            glm::mat4 T = glm::translate(glm::mat4{ 1.0f }, center);
            glm::mat4 R = glm::mat4_cast(rot);
            glm::mat4 S = glm::scale(glm::mat4{ 1.0f }, size);

            m_graphicsManager.gm_PushCubeDebugData(BasicDebugData{ T * R * S });
        }
    }

}