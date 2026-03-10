#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "WinScreenScript.h"
#include "LevelCompleteScript.h"

class EventColliderScript : public TemplateSC {
public:
    enum class EventType {
        WinScreen = 0,
        LevelComplete = 1
    };

    int eventType = 0; // 0 = WinScreen, 1 = LevelComplete

    utility::GUID eventCanvasObject;
    ecs::EntityID eventCanvasID = -1;

    float timer = 0.f;
    float activationDelay = 10.f; // seconds before trigger is active

    void Start() override {
        eventCanvasID = ecsPtr->GetEntityIDFromGUID(eventCanvasObject);

        physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp || nameComp->entityTag != "Player") return;
            if (timer < activationDelay) return;

            switch (static_cast<EventType>(eventType)) {

            case EventType::WinScreen:
                std::cout << "[EventColliderScript] Triggering WinScreen\n";
                if (auto* winScreen = ecsPtr->GetComponent<WinScreenScript>(eventCanvasID)) {
                    winScreen->ShowWinScreen();
                    std::cout << "[EventColliderScript] WinScreen activated\n";
                }
                break;

            case EventType::LevelComplete:
                std::cout << "[EventColliderScript] Triggering LevelComplete\n";
                if (auto* levelComplete = ecsPtr->GetComponent<LevelCompleteScript>(eventCanvasID)) {
                    levelComplete->ShowLevelComplete();
                    std::cout << "[EventColliderScript] LevelComplete activated\n";
                }
                break;

            default:
                std::cout << "[EventColliderScript] Unknown eventType: " << eventType << "\n";
                break;
            }
            });
    }

    void Update() override {
        timer += ecsPtr->m_GetDeltaTime();
    }

    REFLECTABLE(EventColliderScript,
        eventType,
        eventCanvasObject,
        activationDelay,
        timer);
};