#pragma once
#include "ScriptAdapter/TemplateSC.h"

class EnemyManagerScript : public TemplateSC {
public:
	int agentid;
	int enemyHealth;
	float enemyMovementSpeed;

	bool enemyIsAttacking = false;

	float timeBeforeDamageByFlamethrowerAgain = 0.f;

	float enemyAttackRange = 2.5f;
	float enemyChaseRange = 25.f;

	utility::GUID playerToChase;
	ecs::EntityID playerToChaseID;

	utility::GUID enemyHurtboxPrefab;

	utility::GUID enemyHurtboxPosition;
	ecs::EntityID enemyHurtboxPositionID;

	void Start() override {
		playerToChaseID = ecsPtr->GetEntityIDFromGUID(playerToChase);
		enemyHurtboxPositionID = ecsPtr->GetEntityIDFromGUID(enemyHurtboxPosition);
		auto* trans = ecsPtr->GetComponent<TransformComponent>(entity);
		auto* capsule = ecsPtr->GetComponent<CapsuleColliderComponent>(entity);
		navMeshPtr->AddAgent(agentid, entity, trans->WorldTransformation.position, capsule->capsule.radius, capsule->capsule.height);
	}

	void Update() override {
		auto* enemyTransform = ecsPtr->GetComponent<TransformComponent>(entity);
		auto* playerTransform = ecsPtr->GetComponent<TransformComponent>(playerToChaseID);
		auto* enemyHurtboxPositionTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxPositionID);

		if (!playerTransform || !enemyTransform || !enemyHurtboxPositionTransform) {
			LOGGING_WARN("Components not found");
			return;
		}

		// FUCK
		enemyHurtboxPositionTransform->LocalTransformation.position.z = 1.f;

		// CONSTANTLY LOOK AT PLAYER
		glm::vec3 direction = playerTransform->LocalTransformation.position - enemyTransform->LocalTransformation.position;
		direction = glm::normalize(direction);

		// Calculate yaw (rotation around Y axis)
		float yaw = std::atan2(direction.x, direction.z);

		// Calculate pitch (rotation around X axis)
		float pitch = std::asin(-direction.y);

		// Roll is typically 0 for forward-facing directions
		float roll = 0.0f;

		glm::vec3 rotation(0.f, yaw, 0.f);
		glm::vec3 rotationDegrees = glm::degrees(rotation);

		enemyTransform->LocalTransformation.rotation = rotationDegrees;

		if (glm::distance(enemyTransform->LocalTransformation.position, playerTransform->LocalTransformation.position) <= enemyAttackRange) {

			// DONT UNCOMMENT THIS
			//if (!enemyIsAttacking) {
			//	std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

			//	if (enemyHurtbox) {
			//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			//		ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

			//		if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
			//			enemyHurtboxTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxPositionID)->WorldTransformation.position;
			//		}
			//	}
			//}
			//if (!enemyIsAttacking) {
			//	std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

			//	if (enemyHurtbox) {
			//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			//		ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

			//		if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
			//			enemyHurtboxTransform->LocalTransformation.position = enemyTransform->LocalTransformation.position + direction;
			//		}
			//	}
			//}
			std::cout << "Attack" << std::endl;
			enemyIsAttacking = true;
		}

		if (enemyIsAttacking) {
			// NAVMESH STOP FOLLOWING
			// Not calling MoveAgent stops following - SF
			// ADD ENEMY ATTACKING ANIMATION

			// BEFORE CODE WORKS, I TESTED
			// if (CHECK IF ANIMATION OF THE ENEMY IS AFTER THE ENEMY CLAWED OR SOME SHIT (e.g: ANIMATION TIMER IS AT 2s MARK)) {
					//std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

					//if (enemyHurtbox) {
					//	std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					//	ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

					//	if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
					//		enemyHurtboxTransform->LocalTransformation.position = enemyTransform->LocalTransformation.position + direction;
					//	}
					//}
			// }

			// if (CHECK IF ANIMATION IS DONE) {
			//		enemyIsAttacking = false;
			// }
		}
		else if (glm::distance(enemyTransform->LocalTransformation.position, playerTransform->LocalTransformation.position) <= enemyChaseRange) {
			// NAVMESH FOLLOW TOWARDS PLAYER
			navMeshPtr->MoveAgent(agentid, playerTransform->LocalTransformation.position);
			// ADD ENEMY RUNNING ANIMATION
		}

		if (timeBeforeDamageByFlamethrowerAgain >= 0.f) {
			timeBeforeDamageByFlamethrowerAgain -= ecsPtr->m_GetDeltaTime();
		}
	}

	REFLECTABLE(EnemyManagerScript, enemyHealth, enemyMovementSpeed, playerToChase, enemyHurtboxPrefab, enemyHurtboxPosition);
};