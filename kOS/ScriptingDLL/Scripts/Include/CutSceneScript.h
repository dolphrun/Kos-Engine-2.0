#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class CutSceneScript : public TemplateSC {
public:
	
	utility::GUID video_GUID;
	utility::GUID next_Scene;
	utility::GUID current_Scene;
	utility::GUID start_Button;
	utility::GUID quit_Button;
	utility::GUID audio;
	utility::GUID audio_2;
	utility::GUID button_canva;
	float timer = 0.0f;
	void Start() override;
	void Update() override;
	bool hasClicked = false;
	EntityID v_ID, start_buttonID, quit_ButtonID, canva;


	REFLECTABLE(CutSceneScript, video_GUID , current_Scene, next_Scene, start_Button, quit_Button, audio, audio_2, button_canva);
};


inline void CutSceneScript::Start() {
	//EntityID canva = ecsPtr->GetEntityIDFromGUID(button_canva);
	//ecsPtr->SetActive(canva, false);
	 v_ID = ecsPtr->GetEntityIDFromGUID(video_GUID);
	 start_buttonID = ecsPtr->GetEntityIDFromGUID(start_Button);
	 quit_ButtonID = ecsPtr->GetEntityIDFromGUID(quit_Button);
	 canva = ecsPtr->GetEntityIDFromGUID(button_canva);
	 if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
		 for (auto& af : ac->audioFiles) {
			 if (af.audioGUID == audio && af.isSFX) {
				 af.requestPlay = true;
				 break;
			 }
		 }
	 }
}

inline void CutSceneScript::Update() {

	timer += ecsPtr->m_GetDeltaTime();

	bool test = false;
	auto* vc = ecsPtr->GetComponent<ecs::VideoComponent>(v_ID);
	auto* start_bc = ecsPtr->GetComponent<ecs::ButtonComponent>(start_buttonID);
	auto* quit_bc = ecsPtr->GetComponent<ecs::ButtonComponent>(quit_ButtonID);
	
	bool start_isClicked = start_bc->isPressed;
	bool quit_isClicked = quit_bc->isPressed;

	if (timer > 5.f) {
		ecsPtr->SetActive(canva, true);
		vc->pause = true;
		//Pause the audioW
	}


	if (start_isClicked) {
		vc->pause = false;
		EntityID canva = ecsPtr->GetEntityIDFromGUID(button_canva);
		ecsPtr->SetActive(canva, false);
		////vc->pause = false;
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity) ) {
			if (hasClicked == false) {
				hasClicked = true;
				for (auto& af : ac->audioFiles) {
					if (af.audioGUID == audio_2 && af.isSFX) {

						af.requestPlay = true;
						break;
					}
				}
			}
		}

	}

	if (quit_isClicked) {
		Input->InputExitWindow();
	}
	

	
	if ((!vc->playing) || Input->IsKeyTriggered(keys::K)) {
		Scenes->ClearAllScene();
		Scenes->LoadScene(next_Scene);
	}
}

