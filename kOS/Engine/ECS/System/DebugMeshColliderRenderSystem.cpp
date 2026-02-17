#include "Config/pch.h"
#include "DebugMeshColliderRenderSystem.h"

namespace ecs {
    void DebugMeshColliderRenderSystem::Init() {}

    void DebugMeshColliderRenderSystem::Update() {
        const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {
            NameComponent* name = m_ecs.GetComponent<NameComponent>(id);
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            MeshColliderComponent* mesh = m_ecs.GetComponent<MeshColliderComponent>(id);
            MeshFilterComponent* meshFilter = m_ecs.GetComponent<MeshFilterComponent>(id);

            if (name->hide || !mesh || !meshFilter) { continue; }
            if (meshFilter->meshGUID.Empty()) { continue; }

            std::shared_ptr<R_Model> model = m_resourceManager.GetResource<R_Model>(meshFilter->meshGUID);
            if (!model || model->meshes.empty()) { continue; }

            DebugMeshData debugData;
            unsigned short offset = 0;
            for (const auto& m : model->meshes) {
                for (const auto& v : m.vertices) {
                    debugData.vertices.push_back(v.Position);
                }
                for (const auto& i : m.indices) {
                    debugData.indices.push_back(static_cast<unsigned short>(i) + offset);
                }
                offset += static_cast<unsigned short>(m.vertices.size());
            }

            if (debugData.vertices.empty() || debugData.indices.empty()) { continue; }

            glm::vec3 pos = transform->WorldTransformation.position;
            glm::quat rot{ glm::radians(transform->WorldTransformation.rotation) };
            glm::vec3 scale = transform->WorldTransformation.scale;
            debugData.transform = glm::translate(glm::mat4{ 1.0f }, pos) * glm::mat4_cast(rot) * glm::scale(glm::mat4{ 1.0f }, scale);
            debugData.color = glm::vec3{ 0.0f, 1.0f, 0.0f };

            m_graphicsManager.gm_PushDebugMeshData(std::move(debugData));
        }
    }
}