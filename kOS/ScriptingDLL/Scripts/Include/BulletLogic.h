#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class BulletLogic : public TemplateSC {
public:
	int bulletDamage = 2;
	float bulletSpeed = 100.f;
	glm::vec3 direction;

	float timeBeforeDeath = 2.5f;
	float currentTimer = 0.f;

	utility::GUID enemyDeathSfxGUID; //gonna remove this when stuff are fixed

	void Start() override {
		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
				// ADD SFX OF ENEMY DEATH HERE
				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
					for (auto& af : ac->audioFiles) {
						if (af.audioGUID == enemyDeathSfxGUID && af.isSFX) {
							af.requestPlay = true;
							break;
						}
					}
				}

				ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth -= bulletDamage;

				if (ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth <= 0) {
					ecsPtr->DeleteEntity(col.otherEntityID);
				}
			}

			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
				ecsPtr->DeleteEntity(entity);
			}
		});
	}

	void Update() override {
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

	REFLECTABLE(BulletLogic, bulletDamage, bulletSpeed, enemyDeathSfxGUID)
};