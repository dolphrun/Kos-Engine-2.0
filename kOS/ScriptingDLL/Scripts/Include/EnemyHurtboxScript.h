#pragma once
#include "ScriptAdapter/TemplateSC.h"

class EnemyHurtboxScript : public TemplateSC {
public:
	int enemyDamage = 5;

	float timeBeforeDeath = 0.5f;
	float currentTimer = 0.f;

	utility::GUID playerHurtSfxGUID_1;
	utility::GUID playerHurtSfxGUID_2;

	void Start() override {
		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Player") {
				// ADD SFX OF PLAYER GETTING HURT HERE - Done
				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
					std::vector<ecs::AudioFile*> playerHurtSfxPool;

					for (auto& af : ac->audioFiles) {
						if (af.isSFX) {
							playerHurtSfxPool.push_back(&af);
						}
					}

					if (!playerHurtSfxPool.empty()) {
						int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
						std::cout << "[BulletLogic] Random SFX index chosen = " << idx << std::endl;

						playerHurtSfxPool[idx]->requestPlay = true;
					}
				}

				ecsPtr->GetComponent<PlayerManagerScript>(col.otherEntityID)->currPlayerHitPoints -= enemyDamage;

				// Prob redudant I'll comment first to see if it crashes anything
				//if (ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth <= 0) {
				//	// GAME OVER
				//}
			}

			});
	}

	void Update() override {
		if (currentTimer < timeBeforeDeath) {
			currentTimer += ecsPtr->m_GetDeltaTime();

			if (currentTimer >= timeBeforeDeath) {
				ecsPtr->DeleteEntity(entity);
			}
		}
	}

	REFLECTABLE(EnemyHurtboxScript, enemyDamage, playerHurtSfxGUID_1, playerHurtSfxGUID_2)
};