#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FirePowerupManagerScript : public TemplateSC {
public:
	float fireballSpeed = 10.f;
	int fireballDamage = 5;
	glm::vec3 direction;

	void Start() override {
		// ADD SFX OF FIREBALL HERE

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
					// ADD SFX OF FIREBALL BLAST HERE

					enemyScript->enemyHealth -= fireballDamage;

					if (enemyScript->enemyHealth <= 0) {
						ecsPtr->DeleteEntity(col.otherEntityID);
					}

					ecsPtr->DeleteEntity(entity);
					navMeshPtr->RemoveAgent(enemyScript->agentid);
				}
			}

			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Ground" || ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Default") {
				// ADD SFX OF FIREBALL BLAST HERE
				ecsPtr->DeleteEntity(entity);
			}
		});
	}

	void Update() override {
		if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
			tc->LocalTransformation.position += direction * fireballSpeed * ecsPtr->m_GetDeltaTime();
		}
	}


	REFLECTABLE(FirePowerupManagerScript, fireballSpeed, fireballDamage)
};