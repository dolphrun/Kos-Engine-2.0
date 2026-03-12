#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "PlayerManagerScript.h"

class EnemyManagerScript;

class TankAOEScript : public TemplateSC {
public:
	int aoeDamage = 15;
	float timeBeforeDeath = 0.5f;
	float currentTimer = 0.f;

	ecs::EntityID casterID = 0;

	utility::GUID playerHurtSfxGUID_1;
	utility::GUID playerHurtSfxGUID_2;

	void Start() override;
	void Update() override;

	// Added the SFX GUIDs here!
	REFLECTABLE(TankAOEScript, aoeDamage, timeBeforeDeath, playerHurtSfxGUID_1, playerHurtSfxGUID_2)
};

// --- IMPLEMENTATION ---
#include "EnemyManagerScript.h"

inline void TankAOEScript::Start() {
	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

		// Prevent suicide lol
		if (col.otherEntityID == casterID) return;

		auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		if (!nameComp) return;

		//player dmg
		if (nameComp->entityTag == "Player") {
			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
				std::vector<ecs::AudioFile*> playerHurtSfxPool;

				for (auto& af : ac->audioFiles) {
					if (af.isSFX) {
						playerHurtSfxPool.push_back(&af);
					}
				}

				if (!playerHurtSfxPool.empty()) {
					int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
					std::cout << "[TankAOE] Random SFX index chosen = " << idx << std::endl;
					playerHurtSfxPool[idx]->requestPlay = true;
				}
			}

			if (auto* player = ecsPtr->GetComponent<PlayerManagerScript>(col.otherEntityID)) {
				//player->currPlayerHitPoints -= aoeDamage;
				player->TakeDamage(aoeDamage);
			}
		}

		// enemy dmg
		else if (nameComp->entityTag == "Enemy") {
			if (auto* enemy = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {

				enemy->TakeDamage(aoeDamage, "DEFAULT");

				// Stagger other enemies
				if (enemy->shieldHealth <= 0) {
					enemy->TriggerStagger(1.0f);
				}

				if (enemy->enemyHealth <= 0) {
					enemy->Die();
				}
			}
		}
		});
}

inline void TankAOEScript::Update() {
	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);

			// SAFETY NET for double deletion mashallah
			currentTimer = 9999.f;
		}
	}
}