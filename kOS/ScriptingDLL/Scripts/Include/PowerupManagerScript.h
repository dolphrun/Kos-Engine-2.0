#pragma once
#include "ScriptAdapter/TemplateSC.h"

class PowerupManagerScript : public TemplateSC {
public:
	std::string powerupType;
	utility::GUID powerupParticlesGUID;
	ecs::EntityID powerupParticlesEntityID;

	bool powerupActive = true;

	float powerupCurrentTimeToActive = 5.f;
	float powerupMaxTimeToActive = 5.f;

	void Start() override {
		powerupParticlesEntityID = ecsPtr->GetEntityIDFromGUID(powerupParticlesGUID);
	}

	void Update() override {
		if (powerupCurrentTimeToActive < 5.f) {			
			powerupCurrentTimeToActive += ecsPtr->m_GetDeltaTime();

			if (powerupCurrentTimeToActive >= 5.f) {
				powerupActive = true;
				//ecsPtr->SetActive(powerupParticlesEntityID, true);
			}
		}
	}

	void TurnOffPowerup() {
		powerupCurrentTimeToActive = 0.f;
		powerupActive = false;
		//ecsPtr->SetActive(powerupParticlesEntityID, false);
	}

	REFLECTABLE(PowerupManagerScript, powerupType, powerupParticlesGUID);
};