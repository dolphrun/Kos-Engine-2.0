#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FirePowerupManagerScript : public TemplateSC {
public:
	float fireballSpeed = 10.f;
	int fireballDamage = 5;
	float fireballSpin = 10.f;
	glm::vec3 direction;

	utility::GUID fireballSfxGUID;
	utility::GUID explosionSfxGUID;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	utility::GUID fireSplashPrefab;


	void Start() override {
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == fireballSfxGUID && af.isSFX) {
					af.requestPlay = true;
					break;
				}
			}
		}

		for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
			if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
				scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
				break;
			}
		}

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

			auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
			if (!nameComp) return;

			if (nameComp->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {

					glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(col.otherEntityID)->WorldTransformation.position;
					SpawnFireSplash(hitPos);

					if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

						for (auto& af : ac->audioFiles) {
							if (af.audioGUID == explosionSfxGUID && af.isSFX) {
								af.requestPlay = true;
								break;
							}
						}
					}

					ecsPtr->DeleteEntity(entity);
				}
			}

			if (nameComp->entityTag == "Ground" || nameComp->entityTag == "Default") {
				glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(entity)->WorldTransformation.position;
				SpawnFireSplash(hitPos);
				
				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

					for (auto& af : ac->audioFiles) {
						if (af.audioGUID == explosionSfxGUID && af.isSFX) {
							af.requestPlay = true;
							break;
						}
					}
				}

				ecsPtr->DeleteEntity(entity);
			}
			});
	}

	void Update() override {
		if (auto* transform = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
			//transform->LocalTransformation.rotation += glm::vec3(1.f, 0.f, 0.f);
			transform->LocalTransformation.rotation.x += ecsPtr->m_GetDeltaTime() * fireballSpin;
			transform->LocalTransformation.rotation.y += ecsPtr->m_GetDeltaTime() * fireballSpin;
			transform->LocalTransformation.position += direction * ecsPtr->m_GetDeltaTime() * fireballSpeed;
		}

		//if (auto* rb = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity)) {
		//	rb->velocity = direction * fireballSpeed;
		//}
	}

	void SpawnFireSplash(glm::vec3 position) {
		if (fireSplashPrefab.Empty()) return;

		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		ecs::EntityID splashID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireSplashPrefab);

		if (auto* tc = ecsPtr->GetComponent<TransformComponent>(splashID)) {
			tc->LocalTransformation.position = position;
		}
	}

	REFLECTABLE(FirePowerupManagerScript, fireballSpeed, fireballDamage, fireballSfxGUID, fireSplashPrefab, fireballSpin)
};