#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

// FORWARD DECLARATION: Fixes circular dependency
class EnemyManagerScript;

class FireDashVfxScript : public TemplateSC {
public:

	float timeBeforeDeath = 2.f;
	float currentTimer = 0.f;
	int dashDamage = 10;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 100;

	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;

	REFLECTABLE(FireDashVfxScript, timeBeforeDeath, currentTimer, dashDamage)
};

#include "EnemyManagerScript.h"

inline void FireDashVfxScript::Start() {

	for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
		if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
			scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
			break;
		}
	}

	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

		auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		if (!nameComp) return;

		if (nameComp->entityTag == "Enemy") {
			auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
			if (!enemyScript) return;

			enemyScript->TakeDamage(dashDamage, "FIRE");

			// Only stagger if the shield is broken (or never existed)
			if (enemyScript->shieldHealth <= 0) {
				enemyScript->TriggerStagger(0.5f);
			}

			if (enemyScript->enemyHealth <= 0) {
				enemyScript->Die();

			}
		}
		});
}

inline void FireDashVfxScript::Update() {
	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);
		}
	}
}