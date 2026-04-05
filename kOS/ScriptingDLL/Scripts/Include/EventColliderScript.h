#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "WinScreenScript.h"
#include "LevelCompleteScript.h"
#include "FadeTransition.h"

class EventColliderScript : public TemplateSC {
public:
    enum class EventType {
        WinScreen = 0,
        LevelComplete = 1
    };

    bool hasTouched;
    bool timerDone;

    float hasTouchedDelay = 2.0f;
    float startTimer;

    utility::GUID next_Scene;
    utility::GUID transitionUIObject;
    ecs::EntityID transitionUIObjectID = -1;

    int eventType = 0; // 0 = WinScreen, 1 = LevelComplete

    utility::GUID eventCanvasObject;
    ecs::EntityID eventCanvasID = -1;

    float timer = 0.f;
    float activationDelay = 10.f; // seconds before trigger is active

    void Start() override {
        startTimer = 0;
        hasTouched = false;
        timerDone = false;
        eventCanvasID = ecsPtr->GetEntityIDFromGUID(eventCanvasObject);
        transitionUIObjectID = ecsPtr->GetEntityIDFromGUID(transitionUIObject);

        physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp || nameComp->entityTag != "Player") return;
            if (startTimer < activationDelay) return;

            switch (static_cast<EventType>(eventType)) {

            case EventType::WinScreen:
                std::cout << "[EventColliderScript] Triggering WinScreen\n";
                if (auto* winScreen = ecsPtr->GetComponent<WinScreenScript>(eventCanvasID)) {
                    winScreen->ShowWinScreen();
                    std::cout << "[EventColliderScript] WinScreen activated\n";
                }
                break;

            case EventType::LevelComplete:
                std::cout << "[EventColliderScript] WinScreen activated\n";
                hasTouched = true;
                if (auto* transitionScript = ecsPtr->GetComponent<FadeTransition>(transitionUIObjectID)) {
                    transitionScript->TransitionToBlack(1.5f);
                }
                break;

            default:
                std::cout << "[EventColliderScript] Unknown eventType: " << eventType << "\n";
                break;
            }
            });
    }

    void Update() override {

        startTimer += ecsPtr->m_GetDeltaTime();

        if (hasTouched) {
            timer += ecsPtr->m_GetDeltaTime();

            if (timer >= hasTouchedDelay && !timerDone){
                timerDone = true;
                Scenes->ClearAllScene();
                Scenes->LoadScene(next_Scene);
            }
        }
        
    }

    REFLECTABLE(EventColliderScript,
        eventType,
        eventCanvasObject,
        activationDelay,
        timer,
        next_Scene,
        transitionUIObject);
};