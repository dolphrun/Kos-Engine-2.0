#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class MenuPt2Script : public TemplateSC {
public:

	void Start() override;
	void Update() override;
	utility::GUID mainmenuBGMGUID;

	REFLECTABLE(MenuPt2Script, mainmenuBGMGUID);
};


inline void MenuPt2Script::Start() {
	ecsPtr->SetTimeScale(1.0f);
	ecsPtr->SetState(RUNNING);
	Input->HideCursor(false);  // show cursor as soon as this scene starts

	// Play BGM on start
	if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
		for (auto& af : ac->audioFiles) {
			if (af.audioGUID == mainmenuBGMGUID && af.isBGM) {
				af.requestPlay = true;
				break;
			}
		}
	}
}

inline void MenuPt2Script::Update() {
	// nothing needed — cursor stays visible until another scene takes over
	Input->HideCursor(false);
}
