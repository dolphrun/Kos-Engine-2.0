/******************************************************************/
/*!
\file      TransformSystem.cpp
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Sept 29, 2024
\brief   



Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "ECS/ECS.h"
#include "ECS/ecs.h"
#include "TransformSystem.h"
#include "Utility/MathUtility.h"

namespace ecs {
	
	void TransformSystem::Init(){


	}

	void TransformSystem::Update() {
		const auto& entities = m_entities.Data();
		//for (const EntityID id : entities) {
		//	TransformComponent* transformComp = m_ecs.GetComponent<TransformComponent>(id);
		//	if (transformComp->m_haveParent) continue;
		//	
		//	CalculateAllTransform(m_ecs, transformComp);
		//}

		m_ecs.GetWorkers().ParallelFor(entities.begin(), entities.end(),
			[this](EntityID id) {
				TransformComponent* transformComp = m_ecs.GetComponent<TransformComponent>(id);
				if (!transformComp->m_haveParent) {
					CalculateAllTransform(m_ecs, transformComp);
				}
			}
		);
	}

	void TransformSystem::CalculateAllTransform(ECS& ecs, TransformComponent* transformComp, const glm::mat4& parentWorldMtx) {
		if (!transformComp) return;

		CalculateLocalTransformMtx(ecs, transformComp);
		transformComp->transformation = transformComp->m_haveParent ? parentWorldMtx * transformComp->localTransform : transformComp->localTransform;
		utility::DecomposeMtxIntoTRS(transformComp->transformation, transformComp->WorldTransformation.position, transformComp->WorldTransformation.rotation, transformComp->WorldTransformation.scale);
		for (const EntityID childID : transformComp->m_childID) {
			TransformComponent* child = ecs.GetComponent<TransformComponent>(childID);
			if (child) {
				CalculateAllTransform(ecs, child, transformComp->transformation);
			}
		}
	}

	// Ideally not great to constantly convert to radians, data should be stored in radians but due to the restriction of ImGui might be tough to show data in deg
	// Unless TransformComponent is customised
	void TransformSystem::CalculateLocalTransformMtx(ECS& ecs, TransformComponent* transformComp) {
		if (!transformComp) return;
		constexpr glm::mat4 identity(1.0f);
		transformComp->localTransform = glm::translate(identity, transformComp->LocalTransformation.position) *
										glm::mat4_cast(glm::quat(glm::radians(transformComp->LocalTransformation.rotation))) *
										glm::scale(identity, transformComp->LocalTransformation.scale);
	}

	void TransformSystem::SetImmediateWorldPosition(ECS& ecs, TransformComponent* transformComp, glm::vec3&& pos){
		if (!transformComp) return;
		constexpr glm::mat4 identity(1.0f);
		transformComp->WorldTransformation.position = pos;
		transformComp->transformation = glm::translate(identity, transformComp->WorldTransformation.position) *
										glm::mat4_cast(glm::quat(glm::radians(transformComp->WorldTransformation.rotation))) *
										glm::scale(identity, transformComp->WorldTransformation.scale);

		TransformComponent* parentTrans = ecs.GetComponent<TransformComponent>(transformComp->m_parentID);
		if (parentTrans) {
			transformComp->localTransform = glm::inverse(parentTrans->transformation) * transformComp->transformation;
			utility::DecomposeMtxIntoTRS(transformComp->localTransform, transformComp->LocalTransformation.position, transformComp->LocalTransformation.rotation, transformComp->LocalTransformation.scale);
		}
		else {
			transformComp->LocalTransformation.position = transformComp->WorldTransformation.position;
			transformComp->localTransform = transformComp->transformation;
		}
	}

	void TransformSystem::SetImmediateWorldRotation(ECS& ecs, TransformComponent* transformComp, glm::vec3&& rot){
		if (!transformComp) return;
		constexpr glm::mat4 identity(1.0f);
		transformComp->WorldTransformation.rotation = rot;
		transformComp->transformation = glm::translate(identity, transformComp->WorldTransformation.position) *
										glm::mat4_cast(glm::quat(glm::radians(transformComp->WorldTransformation.rotation))) *
										glm::scale(identity, transformComp->WorldTransformation.scale);

		TransformComponent* parentTrans = ecs.GetComponent<TransformComponent>(transformComp->m_parentID);
		if (parentTrans) {
			transformComp->localTransform = glm::inverse(parentTrans->transformation) * transformComp->transformation;
			utility::DecomposeMtxIntoTRS(transformComp->localTransform, transformComp->LocalTransformation.position, transformComp->LocalTransformation.rotation, transformComp->LocalTransformation.scale);
		}
		else {
			transformComp->LocalTransformation.rotation = transformComp->WorldTransformation.rotation;
			transformComp->localTransform = transformComp->transformation;
		}
	}

	void TransformSystem::SetImmediateWorldScale(ECS& ecs, TransformComponent* transformComp, glm::vec3&& scale){
		if (!transformComp) return;
		constexpr glm::mat4 identity(1.0f);
		transformComp->WorldTransformation.scale = scale;
		transformComp->transformation = glm::translate(identity, transformComp->WorldTransformation.position) *
			glm::mat4_cast(glm::quat(glm::radians(transformComp->WorldTransformation.rotation))) *
			glm::scale(identity, transformComp->WorldTransformation.scale);

		TransformComponent* parentTrans = ecs.GetComponent<TransformComponent>(transformComp->m_parentID);
		if (parentTrans) {
			transformComp->localTransform = glm::inverse(parentTrans->transformation) * transformComp->transformation;
			utility::DecomposeMtxIntoTRS(transformComp->localTransform, transformComp->LocalTransformation.position, transformComp->LocalTransformation.rotation, transformComp->LocalTransformation.scale);
		}
		else {
			transformComp->LocalTransformation.scale = transformComp->WorldTransformation.scale;
			transformComp->localTransform = transformComp->transformation;
		}
	}

	void TransformSystem::SetImmediateLocalPosition(ECS& ecs, TransformComponent* transformComp, glm::vec3&& pos){
		if (!transformComp) return;
		transformComp->LocalTransformation.position = pos;
		CalculateAllTransform(ecs,transformComp);
	}

	void TransformSystem::SetImmediateLocalRotation(ECS& ecs, TransformComponent* transformComp, glm::vec3&& rot){
		if (!transformComp) return;
		transformComp->LocalTransformation.rotation = rot;
		CalculateAllTransform(ecs,transformComp);
	}

	void TransformSystem::SetImmediateLocalScale(ECS& ecs, TransformComponent* transformComp, glm::vec3&& scale){
		if (!transformComp) return;
		transformComp->LocalTransformation.scale = scale;
		CalculateAllTransform(ecs, transformComp);
	}
}
