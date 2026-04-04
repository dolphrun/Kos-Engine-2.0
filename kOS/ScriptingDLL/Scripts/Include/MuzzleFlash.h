#pragma once

class MuzzleFlash : public TemplateSC {
public:
	float timeBeforeDeath = 0.35f;
	float currentTimer = 0.f;

	void Update() override {
		if (currentTimer < timeBeforeDeath) {
			currentTimer += ecsPtr->m_GetDeltaTime();

			if (currentTimer >= timeBeforeDeath) {
				ecsPtr->DeleteEntity(entity);

			}
		}
	}

	REFLECTABLE(MuzzleFlash)
};