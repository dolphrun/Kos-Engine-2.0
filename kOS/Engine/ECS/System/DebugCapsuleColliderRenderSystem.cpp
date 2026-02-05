#include "Config/pch.h"
#include "DebugCapsuleColliderRenderSystem.h"
#include "Graphics/GraphicsManager.h"
#include "Resources/ResourceManager.h"

namespace ecs {

    void DebugCapsuleColliderRenderSystem::Init() {}

    void DebugCapsuleColliderRenderSystem::Update() {
        const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            CapsuleColliderComponent* capsule = m_ecs.GetComponent<CapsuleColliderComponent>(id);
            if (!transform || !capsule) { continue; }
            glm::vec3 scale = transform->WorldTransformation.scale;
            glm::vec3 center = capsule->capsule.center * scale + transform->WorldTransformation.position;
            glm::mat4 rotationMatrix = glm::mat4{ 1.0f };
            float radiusScale = 1.0f;
            float heightScale = 1.0f;
            switch (capsule->capsule.capsuleDirection) {
            case CapsuleDirection::X:
                rotationMatrix = glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3{ 0.0f, 0.0f, 1.0f });
                radiusScale = std::max(scale.y, scale.z);
                heightScale = scale.x;
                break;
            case CapsuleDirection::Y:
                radiusScale = std::max(scale.x, scale.z);
                heightScale = scale.y;
                break;
            case CapsuleDirection::Z:
                rotationMatrix = glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
                radiusScale = std::max(scale.x, scale.y);
                heightScale = scale.z;
                break;
            }
            glm::mat4 rot = glm::mat4_cast(glm::quat(glm::radians(transform->WorldTransformation.rotation)));
            float r = capsule->capsule.radius * radiusScale;
            float h = capsule->capsule.height * heightScale;
            h = glm::max(h, 2.0f * r);
            glm::mat4 result = glm::translate(glm::mat4{ 1.0f }, center) * rot * rotationMatrix;
            BasicDebugData d;
            d.worldTransform = result;
            d.color = glm::vec3{ 0.0f, 1.0f, 0.0f };
            d.radius = r;
            d.height = h; 
            m_graphicsManager.gm_PushCapsuleDebugData(std::move(d));
        }
    }
}