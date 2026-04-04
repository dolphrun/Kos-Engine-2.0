#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class CutsceneAutoScript : public TemplateSC {
public:

	utility::GUID video_GUID;
	utility::GUID audio;
	utility::GUID next_Scene;

	void Start() override;
	void Update() override;
	EntityID v_ID;
	bool cutsceneEnded = false;
	REFLECTABLE(CutsceneAutoScript, video_GUID, audio, next_Scene);
};


inline void CutsceneAutoScript::Start() {
	v_ID = ecsPtr->GetEntityIDFromGUID(video_GUID);
	if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
		for (auto& af : ac->audioFiles) {
			if (af.audioGUID == audio && af.isSFX) {
				af.requestPlay = true;
				break;
			}
		}
	}
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