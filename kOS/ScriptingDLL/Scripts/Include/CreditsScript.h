#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class CreditsScript : public TemplateSC {
public:
	utility::GUID next_Scene;
	float creditsDuration = 10.0f;  // Set this in the inspector to match your video length
	float timer = 0.0f;

	void Start() override;
	void Update() override;

	REFLECTABLE(CreditsScript, creditsDuration, next_Scene);
};


inline void CreditsScript::Start() {
	timer = 0.0f;
}

inline void CreditsScript::Update() {
	timer += ecsPtr->m_GetDeltaTime();
	if (timer >= creditsDuration) {
		Scenes->LoadScene(next_Scene);
	}
}
