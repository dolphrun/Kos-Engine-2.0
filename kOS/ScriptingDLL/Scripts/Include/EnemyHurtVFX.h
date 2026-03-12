#pragma once

class EnemyHurtVFX : public TemplateSC {
public:

	float currentTimer = 0.f;
	float timeBeforeDeath = 5.f;


	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;

	REFLECTABLE(EnemyHurtVFX)
};

inline void EnemyHurtVFX::Start() {

}

inline void EnemyHurtVFX::Update() {
	if (currentTimer < timeBeforeDeath) {
		currentTimer += ecsPtr->m_GetDeltaTime();

		if (currentTimer >= timeBeforeDeath) {
			ecsPtr->DeleteEntity(entity);

		}
	}
}