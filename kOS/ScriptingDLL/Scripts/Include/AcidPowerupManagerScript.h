#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class AcidPowerupManagerScript : public TemplateSC {
public:
	//int acidDamage = 5;
	float pushbackForce = 500.f;
	float lingerTime;
	float growthRate;
	float acidBlastSpeed;

	float currentTimer = 0.f;
	glm::vec3 direction;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	utility::GUID acidSpraySfxGUID;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	void Start() override {
		enemyDeathSfxGUIDs.clear();
		if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
		if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
		if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

		// ADD SFX OF ACID SPRAY HERE - Done
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == acidSpraySfxGUID && af.isSFX) {
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
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
					//enemyScript->enemyHealth -= acidDamage;
					//std::cout << "AI SHIBAL ACID\n";
					//if (enemyScript->enemyHealth <= 0) {
					//	// ADD SFX OF ENEMY DEATH HERE - Done
					//	PlayRandomEnemyDeathSFX();

					//	if (scoreManager) {
					//		scoreManager->AddScore(scoreValue); // or whatever value you want per kill
					//	}

					//	ecsPtr->DeleteEntity(col.otherEntityID);
					//	navMeshPtr->RemoveAgent(enemyScript->agentid);
					//}

					//auto* enemyRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(col.otherEntityID);
					//if (!enemyRigidbody) return;

					//physicsPtr->AddForce(enemyRigidbody->actor, direction * pushbackForce, ForceMode::Impulse);
					//std::cout << "ENEMY PUSH BACK\n";
				}

				auto* enemyRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(col.otherEntityID);
				if (!enemyRigidbody) return;

				glm::vec3 force = direction * pushbackForce;
				physicsPtr->AddForce(enemyRigidbody->actor, force, ForceMode::Impulse);
				std::cout << "ENEMY PUSH BACK: " << force.x << ", " << force.y << ", " << force.z << std::endl;
			}
			});
	}

	void Update() override {
		if (auto* rb = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity)) {
			rb->velocity = direction * acidBlastSpeed;
		}

		if (currentTimer <= lingerTime) {
			currentTimer += ecsPtr->m_GetDeltaTime();
			ecsPtr->GetComponent<TransformComponent>(entity)->LocalTransformation.scale *= growthRate;

			if (currentTimer >= lingerTime) {
				ecsPtr->DeleteEntity(entity);
			}
		}
	}

	void PlayRandomEnemyDeathSFX()
	{
		auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity);
		if (!ac) return;

		std::vector<ecs::AudioFile*> enemyDeathSfxPool;

		for (auto& af : ac->audioFiles) {
			if (!af.isSFX) continue;

			for (auto& g : enemyDeathSfxGUIDs) {
				if (!g.Empty() && af.audioGUID == g) {
					enemyDeathSfxPool.push_back(&af);
					break; 
				}
			}
		}

		if (enemyDeathSfxPool.empty()) {
			std::cout << "[BulletLogic] No enemy death SFX found.\n";
			return;
		}

		int idx = rand() % static_cast<int>(enemyDeathSfxPool.size());
		enemyDeathSfxPool[idx]->requestPlay = true;

		std::cout << "[BulletLogic] Playing enemy death SFX index " << idx << "\n";
	}

	REFLECTABLE(AcidPowerupManagerScript, pushbackForce, lingerTime, growthRate, acidBlastSpeed, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3, acidSpraySfxGUID)
};