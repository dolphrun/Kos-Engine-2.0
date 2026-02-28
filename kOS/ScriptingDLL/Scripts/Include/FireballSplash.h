#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FireballSplash : public TemplateSC {
public:
	int splashDamage = 40;        
	float splashRadius = 5.f; 

	utility::GUID splashSfxGUID;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	float timeBeforeDeath = 2.f;
	float currentTimer = 0.f;

	bool hasDealtDamage = false;


	void Start() override {
		// ADD SFX OF FIREBALL HERE - Done?
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == splashSfxGUID && af.isSFX) {
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

		auto damagedEnemies = std::make_shared<std::unordered_set<ecs::EntityID>>();

		physicsPtr->GetEventCallback()->OnTriggerStay(entity, [this, damagedEnemies](const physics::Collision& col) {

			if (damagedEnemies->count(col.otherEntityID)) return;

			auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
			if (!nameComp || nameComp->entityTag != "Enemy") return;

			auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
			if (!enemyScript) return;

			damagedEnemies->insert(col.otherEntityID);

			enemyScript->enemyHealth -= splashDamage;
			std::cout << "[FireballSplash] Hit enemy, HP left: " << enemyScript->enemyHealth << "\n";

			if (enemyScript->enemyHealth <= 0) {
				if (scoreManager) {
					scoreManager->AddScore(scoreValue);
				}
				navMeshPtr->RemoveAgent(enemyScript->agentid); 
				ecsPtr->DeleteEntity(col.otherEntityID);
			}
			});

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

		//physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

		//	auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		//	if (!nameComp) return;

		//	if (nameComp->entityTag == "Enemy") {
		//		if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {

		//			glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(col.otherEntityID)->WorldTransformation.position;
		//			SpawnFireSplash(hitPos);

		//			ecsPtr->DeleteEntity(entity);
		//		}
		//	}

		//	if (nameComp->entityTag == "Ground" || nameComp->entityTag == "Default") {
		//		glm::vec3 hitPos = ecsPtr->GetComponent<ecs::TransformComponent>(entity)->WorldTransformation.position;
		//		SpawnFireSplash(hitPos);

		//		ecsPtr->DeleteEntity(entity);
		//	}
		//	});
	}

	void Update() override {
		if (currentTimer < timeBeforeDeath) {
			currentTimer += ecsPtr->m_GetDeltaTime();

			if (currentTimer >= timeBeforeDeath) {
				ecsPtr->DeleteEntity(entity);
			}
		}
	}

	REFLECTABLE(FireballSplash, splashDamage, splashSfxGUID, scoreValue, timeBeforeDeath)
};