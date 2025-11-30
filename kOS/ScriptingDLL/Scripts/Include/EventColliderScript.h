#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class EventColliderScript : public TemplateSC {
public:
	utility::GUID winUIObject;
	ecs::EntityID winUIObjectID;


	utility::GUID winScreenCanvasObject;
	ecs::EntityID winScreenCanvasID;

	float timer;

	void Start() {
		winUIObjectID = ecsPtr->GetEntityIDFromGUID(winUIObject);
		winScreenCanvasID = ecsPtr->GetEntityIDFromGUID(winScreenCanvasObject);

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Player" && timer > 10.f) {
				std::cout << "LEVEL COMPLETE\n";
				if (auto* WinScreen = ecsPtr->GetComponent<WinScreenScript>(winScreenCanvasID)) {
					WinScreen->ShowWinScreen();
					std::cout << "WIN SCREEN ACTIVATED\n";
					return;
				}
			}
			});
	}

	void Update() {
		timer += ecsPtr->m_GetDeltaTime();

		
	}

	REFLECTABLE(EventColliderScript, winUIObject, winUIObjectID, timer, winScreenCanvasObject);
};