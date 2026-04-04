#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class CutsceneAutoScript : public TemplateSC {
public:

	utility::GUID video_GUID;
	utility::GUID next_Scene;

	void Start() override;
	void Update() override;
	EntityID v_ID;
	bool cutsceneEnded = false;
	REFLECTABLE(CutsceneAutoScript, video_GUID, next_Scene);
};


inline void CutsceneAutoScript::Start() {
	v_ID = ecsPtr->GetEntityIDFromGUID(video_GUID);

	//Turn off mouse
	Input->HideCursor(true);
}

inline void CutsceneAutoScript::Update() {
	if (cutsceneEnded) return;
	auto* vc = ecsPtr->GetComponent<ecs::VideoComponent>(v_ID);

	if ((!vc->playing) || Input->IsKeyTriggered(keys::K)) {
		if (!next_Scene.Empty()) {
			cutsceneEnded = true;
			Scenes->ClearAllScene();
			Scenes->LoadScene(next_Scene);
		}
	}
}