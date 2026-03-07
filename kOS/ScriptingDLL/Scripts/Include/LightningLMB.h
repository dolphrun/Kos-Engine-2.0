#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"


// FORWARD DECLARATION: Fixes circular dependency
class EnemyManagerScript;

class LightningLMB : public TemplateSC {
public:

	//Pew pew pref
	glm::vec3 direction;
	float projectileSpeed = 50.0f;

	float timeBeforeDeath = 10.5f;
	float currentTimer = 0.f;

	// needler stacking
	int lightningLMBDamage = 1;         
	int stacksPerExplode = 3; //stacks before BOMB
	int explodeDamage = 5; // big PP damage

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

	REFLECTABLE(LightningLMB, lightningLMBDamage, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3)
};

// --- IMPLEMENTATION SECTION ---
// Include EnemyManagerScript here so we can access enemyHealth
#include "EnemyManagerScript.h"

inline void LightningLMB::Start() {
	enemyDeathSfxGUIDs.clear();
	if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
	if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
	if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

	for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
		if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
			scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
			break;
		}
	}



	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {

			// FETCH ENEMY SCRIPT ONCE
			auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
			if (!enemyScript) return;

			// Lightning stack less than stacks per explode
			if (enemyScript->lightningStack < stacksPerExplode) {
				// APPLY DAMAGE VIA TAKEDAMAGE
				enemyScript->TakeDamage(lightningLMBDamage, "LIGHTNING");
				enemyScript->lightningStack += lightningLMBDamage;
			}
			else {
				// EXPLOSION DAMAGE
				enemyScript->TakeDamage(explodeDamage, "LIGHTNING");
				enemyScript->lightningStack = 0;
			}

			if (enemyScript->enemyHealth <= 0) {
				// ADD SFX OF ENEMY DEATH HERE - DONE
				PlayRandomEnemyDeathSFX();

				if (scoreManager) {
					scoreManager->AddScore(scoreValue);
				}

				enemyScript->Die();
			}
			else if (enemyScript->shieldHealth <= 0 && enemyScript->lightningStack == 0) {
				// I changed to only stagger when blow up can revert if yall want
				enemyScript->TriggerStagger(0.5f);
			}
		}

		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
			ecsPtr->DeleteEntity(entity);
		}
	});
}

inline void LightningLMB::Update() {

	if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
		tc->LocalTransformation.position += direction * projectileSpeed * ecsPtr->m_GetDeltaTime();
	}

	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);

		}
	}
}

inline void LightningLMB::PlayRandomEnemyDeathSFX()
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
		std::cout << "[LightningLMB] No enemy death SFX found.\n";
		return;
	}

	int idx = rand() % static_cast<int>(enemyDeathSfxPool.size());
	enemyDeathSfxPool[idx]->requestPlay = true;

	std::cout << "[LightningLMB] Playing enemy death SFX index " << idx << "\n";
}