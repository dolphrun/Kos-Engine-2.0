#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class SplashScreenScript : public TemplateSC {
public:

	bool stopUpdate = false;

	utility::GUID next_Scene;
	float timer = 0.0f;
	void Start() override;
	void Update() override;

	REFLECTABLE(SplashScreenScript, next_Scene);
};


inline void SplashScreenScript::Start(){

}

inline void SplashScreenScript::Update() {
	//ecsPtr->SetActive(next_Scene_ID, false);
	timer += ecsPtr->m_GetDeltaTime();
	if (timer > 5.0f && !stopUpdate) {

		Scenes->LoadScene(next_Scene);
		Scenes->ClearAllScene();
		stopUpdate = true;
	}
}
