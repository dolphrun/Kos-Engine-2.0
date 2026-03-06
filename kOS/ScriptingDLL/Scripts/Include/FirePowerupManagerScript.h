#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FirePowerupManagerScript : public TemplateSC {
public:
	float fireballSpeed = 10.f;
	int fireballDamage = 5;
	glm::vec3 direction;

	utility::GUID fireballSfxGUID;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	utility::GUID fireSplashPrefab;


	void Start() override {
		// ADD SFX OF FIREBALL HERE - Done?
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

		//physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
		//	if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
		//		if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
		//			// ADD SFX OF FIREBALL BLAST HERE

		//			enemyScript->enemyHealth -= fireballDamage;

		//			if (enemyScript->enemyHealth <= 0) {

		//				if (scoreManager) {
		//					scoreManager->AddScore(scoreValue); // or whatever value you want per kill
		//				}

		//				ecsPtr->DeleteEntity(col.otherEntityID);
		//			}

		//			ecsPtr->DeleteEntity(entity);
		//			navMeshPtr->RemoveAgent(enemyScript->agentid);
		//		}
		//	}

		//	if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
		//		// ADD SFX OF FIREBALL BLAST HERE
		//		ecsPtr->DeleteEntity(entity);
		//	}
		//});

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

			auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
			if (!nameComp) return;

			if (nameComp->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {

					glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(col.otherEntityID)->WorldTransformation.position;
					SpawnFireSplash(hitPos);

					ecsPtr->DeleteEntity(entity);
				}
			}

			if (nameComp->entityTag == "Ground" || nameComp->entityTag == "Default") {
				glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(entity)->WorldTransformation.position;
				SpawnFireSplash(hitPos);

				ecsPtr->DeleteEntity(entity);
			}
			});
	}

	void Update() override {
		if (auto* rb = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity)) {
			rb->velocity = direction * fireballSpeed;
		}
	}

	void SpawnFireSplash(glm::vec3 position) {
		if (fireSplashPrefab.Empty()) return;

		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		ecs::EntityID splashID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireSplashPrefab);

		if (auto* tc = ecsPtr->GetComponent<TransformComponent>(splashID)) {
			tc->LocalTransformation.position = position;
		}
	}


	REFLECTABLE(FirePowerupManagerScript, fireballSpeed, fireballDamage, fireballSfxGUID, fireSplashPrefab)
};