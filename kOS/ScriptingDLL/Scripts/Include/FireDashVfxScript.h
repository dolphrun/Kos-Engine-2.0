#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

// FORWARD DECLARATION: Fixes circular dependency


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

// --- IMPLEMENTATION SECTION ---


inline void FireDashVfxScript::Start() {

	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

		auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		if (!nameComp) return;

		if (nameComp->entityTag == "Enemy") {
			auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
			if (!enemyScript) return;

			enemyScript->enemyHealth -= dashDamage;
			enemyScript->TriggerStagger(1.f);

			if (enemyScript->enemyHealth <= 0) {
				enemyScript->Die();

				if (scoreManager) {
					scoreManager->AddScore(scoreValue);
				}

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