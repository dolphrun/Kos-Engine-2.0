#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class LightningPowerupManagerScript : public TemplateSC {
public:
	float lightningSpeed = 5.f;
	int lightningDamage = 15;
	glm::vec3 direction;

	float lingerTime = 1.f;
	float currentTimer = 0.f;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	utility::GUID lightningStrikeSfxGUID;

	void Start() override {
		// ADD SFX OF LIGHTNING STRIKE HERE - Done
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == lightningStrikeSfxGUID && af.isSFX) {
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
					enemyScript->enemyHealth -= lightningDamage;

					if (enemyScript->enemyHealth <= 0) {
						// ADD SFX OF ENEMY DEATH HERE - Done
						PlayRandomEnemyDeathSFX();

						if (scoreManager) {
							scoreManager->AddScore(scoreValue); // or whatever value you want per kill
						}

						ecsPtr->DeleteEntity(col.otherEntityID);
						navMeshPtr->RemoveAgent(enemyScript->agentid);
					}
				}
			}
		});


		//glm::vec3 dir = glm::normalize(direction);
		//glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f); // bullet default forward
		//glm::quat rotation = glm::rotation(forward, dir);
		////glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(rotation));
		//glm::vec3 eulerDeg = glm::eulerAngles(rotation);
		//ecsPtr->GetComponent<ecs::TransformComponent>(entity)->LocalTransformation.rotation = eulerDeg;

		glm::vec3 dir = glm::normalize(direction);
		float yaw = atan2(dir.x, dir.z);   // left/right
		float pitch = asin(-dir.y);          // up/down
		glm::vec3 rotationDegrees;
		rotationDegrees.x = glm::degrees(pitch) + 90.f; // Pitch (X)
		rotationDegrees.y = glm::degrees(yaw);     // Yaw (Y)
		rotationDegrees.z = 0.0f;            // Roll
		ecsPtr->GetComponent<ecs::TransformComponent>(entity)->LocalTransformation.rotation = rotationDegrees;

	}

	void Update() override {
		//if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
		//	tc->LocalTransformation.position += direction * lightningSpeed * ecsPtr->m_GetDeltaTime();
		//}

		if (currentTimer <= lingerTime) {
			currentTimer += ecsPtr->m_GetDeltaTime();

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

	REFLECTABLE(LightningPowerupManagerScript, lightningDamage, lingerTime, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3, lightningStrikeSfxGUID)
};