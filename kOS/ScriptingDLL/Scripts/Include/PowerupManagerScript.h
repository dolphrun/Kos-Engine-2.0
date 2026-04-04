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

	//glm::vec3 originalPos;
	//glm::vec3 narnia{ 1000.f, 1000.f, 1000.f };

	void Start() override;

	void Update() override;

	void TurnOffPowerup();

	REFLECTABLE(PowerupManagerScript, powerupType, powerupParticlesGUID);
};

inline void PowerupManagerScript::Start() {

}

inline void PowerupManagerScript::Update() {
	if (powerupCurrentTimeToActive < 5.f) {
		powerupCurrentTimeToActive += ecsPtr->m_GetDeltaTime();

		if (powerupCurrentTimeToActive >= 5.f) {
			powerupActive = true;

			//powerupParticlesEntityID = ecsPtr->GetEntityIDFromGUID(powerupParticlesGUID);
			std::vector<EntityID> children = ecsPtr->GetChild(entity).value();
			if (children[0]) {
				ecsPtr->SetActive(children[0], true);
			}
		}
	}
}

inline 	void PowerupManagerScript::TurnOffPowerup() {
	powerupCurrentTimeToActive = 0.f;
	powerupActive = false;

	//powerupParticlesEntityID = ecsPtr->GetEntityIDFromGUID(powerupParticlesGUID)
	std::vector<EntityID> children = ecsPtr->GetChild(entity).value();
	if (children[0]) {
		ecsPtr->SetActive(children[0], false);
	}
}