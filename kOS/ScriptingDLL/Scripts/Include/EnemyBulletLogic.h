#pragma once
#include "ScriptAdapter/TemplateSC.h"

// FORWARD DECLARATION: Fixes circular dependency
class PlayerManagerScript;

class EnemyBulletLogic : public TemplateSC {
public:
	int bulletDamage = 1;
	float bulletSpeed = 80.f;
	glm::vec3 direction;

	float timeBeforeDeath = 2.5f;
	float currentTimer = 0.f;

	utility::GUID playerHurtSfxGUID_1;
	utility::GUID playerHurtSfxGUID_2;

	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;

	REFLECTABLE(EnemyBulletLogic, bulletDamage, bulletSpeed, playerHurtSfxGUID_1, playerHurtSfxGUID_2)
};

// --- IMPLEMENTATION SECTION ---
#include "PlayerManagerScript.h"

inline void EnemyBulletLogic::Start() {
	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Player") {

			// Audio implementation for player sfx from hurtbox
			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
				std::vector<ecs::AudioFile*> playerHurtSfxPool;

				for (auto& af : ac->audioFiles) {
					if (af.isSFX) {
						playerHurtSfxPool.push_back(&af);
					}
				}

				if (!playerHurtSfxPool.empty()) {
					int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
					std::cout << "[EnemyBulletLogic] Random SFX index chosen = " << idx << std::endl;

					playerHurtSfxPool[idx]->requestPlay = true;
				}
			}

			// Deal damage
			ecsPtr->GetComponent<PlayerManagerScript>(col.otherEntityID)->currPlayerHitPoints -= bulletDamage;
		}

		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
			ecsPtr->DeleteEntity(entity);
		}
	});
}

inline void EnemyBulletLogic::Update() {
	if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
		tc->LocalTransformation.position += direction * bulletSpeed * ecsPtr->m_GetDeltaTime();
	}

	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);
		}
	}
}