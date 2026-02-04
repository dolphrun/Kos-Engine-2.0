#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

// FORWARD DECLARATION: Fixes circular dependency
class EnemyManagerScript;

class FireLMB : public TemplateSC {
public:
	int fireLMBDamage = 4;
	glm::vec3 direction;

	float timeBeforeDeath = 0.25f;
	float currentTimer = 0.f;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 100;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;
	void PlayRandomEnemyDeathSFX();

	REFLECTABLE(FireLMB, fireLMBDamage, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3)
};

// --- IMPLEMENTATION SECTION ---
// Include EnemyManagerScript here so we can access enemyHealth
#include "EnemyManagerScript.h"

inline void FireLMB::Start() {
	enemyDeathSfxGUIDs.clear();
	if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
	if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
	if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3); \

		for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
			if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
				scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
				break;
			}
		}

	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
			// ADD SFX OF ENEMY DEATH HERE - DONE
			PlayRandomEnemyDeathSFX();

			/*			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
							std::vector<ecs::AudioFile*> candidates;

							for (auto& af : ac->audioFiles) {
								if (af.isSFX) {
									candidates.push_back(&af);
								}
							}

							if (!candidates.empty()) {
								int idx = rand() % static_cast<int>(candidates.size());
								std::cout << "[BulletLogic] Random SFX index chosen = " << idx << std::endl;

								candidates[idx]->requestPlay = true;
							}
						}*/
						//if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

						//	for (auto& af : ac->audioFiles) {
						//		if (af.audioGUID == enemyDeathSfxGUID_1 && af.isSFX) {
						//			af.requestPlay = true;
						//			break;
						//		}
						//	}
						//}

			ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth -= fireLMBDamage;

			if (ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth <= 0) {
				if (scoreManager) {
					scoreManager->AddScore(scoreValue); // or whatever value you want per kill
				}

				ecsPtr->DeleteEntity(col.otherEntityID);
				navMeshPtr->RemoveAgent(ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->agentid);
			}
		}

		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
			ecsPtr->DeleteEntity(entity);
		}
		});
}

inline void FireLMB::Update() {
	if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
		tc->LocalTransformation.position += direction * ecsPtr->m_GetDeltaTime();
	}

	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);

		}
	}
}

inline void FireLMB::PlayRandomEnemyDeathSFX()
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
		std::cout << "[FireLMB] No enemy death SFX found.\n";
		return;
	}

	int idx = rand() % static_cast<int>(enemyDeathSfxPool.size());
	enemyDeathSfxPool[idx]->requestPlay = true;

	std::cout << "[FireLMB] Playing enemy death SFX index " << idx << "\n";
}