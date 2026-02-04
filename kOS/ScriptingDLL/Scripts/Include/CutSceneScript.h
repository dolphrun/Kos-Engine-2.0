#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class CutSceneScript : public TemplateSC {
public:
	
	utility::GUID video_GUID;
	utility::GUID next_Scene;
	utility::GUID current_Scene;


	void Start() override;
	void Update() override;


	REFLECTABLE(CutSceneScript, video_GUID , current_Scene, next_Scene);
};


inline void CutSceneScript::Start() {

	

}

inline void CutSceneScript::Update() {
	EntityID v_ID = ecsPtr->GetEntityIDFromGUID(video_GUID);
	auto* vc = ecsPtr->GetComponent<ecs::VideoComponent>(v_ID);

	if ((!vc->playing && !vc->loop) || Input->IsKeyTriggered(keys::K) ) {
		Scenes->ClearAllScene();
		Scenes->LoadScene(next_Scene);
	}
}

