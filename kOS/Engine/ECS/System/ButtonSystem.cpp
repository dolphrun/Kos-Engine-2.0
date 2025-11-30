#include "Config/pch.h"
#include "ECS/ECS.h"

#include "ButtonSystem.h"
#include "Graphics/GraphicsManager.h"
namespace ecs {

	void ButtonSystem::Init() {


	}

	void ButtonSystem::Update() {
		
		//Do AABB calculation
		const auto& entities = m_entities.Data();
		for (const EntityID id : entities) {
			TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
		}

	}


}
