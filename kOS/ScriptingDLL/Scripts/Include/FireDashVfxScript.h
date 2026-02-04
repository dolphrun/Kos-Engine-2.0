#pragma once
#include "ScriptAdapter/TemplateSC.h"

// FORWARD DECLARATION: Fixes circular dependency


class FireDashVfxScript : public TemplateSC {
public:

	float timeBeforeDeath = 2.f;
	float currentTimer = 0.f;


	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;


	REFLECTABLE(FireDashVfxScript, timeBeforeDeath, currentTimer)
};

// --- IMPLEMENTATION SECTION ---


inline void FireDashVfxScript::Start() {

}

inline void FireDashVfxScript::Update() {


	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);

		}
	}
}