#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include <iostream>

class LevelCompleteScript : public TemplateSC {
public:
    static bool isLevelCompleteActive;
    static LevelCompleteScript* instance;

    utility::GUID levelCompleteCanvasGUID;
    ecs::EntityID levelCompleteCanvasID = 0;

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    bool hasShownLevelComplete = false;

    void Start() override {
        instance = this;
        isLevelCompleteActive = false;
        levelCompleteCanvasID = ecsPtr->GetEntityIDFromGUID(levelCompleteCanvasGUID);

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(levelCompleteCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }

        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        SetLevelCompleteActive(false);
    }

    void Update() override {
        // Nothing needed here
    }

    void ShowLevelComplete() {
        if (hasShownLevelComplete)
            return;

        hasShownLevelComplete = true;
        isLevelCompleteActive = true;
        ecsPtr->SetTimeScale(0.0f);
        ecsPtr->SetState(WAIT);
        SetLevelCompleteActive(true);
        Input->HideCursor(false);

        std::cout << "LEVEL COMPLETE SHOWN\n";
    }

    void HideLevelComplete() {
        

        hasShownLevelComplete = false;
        isLevelCompleteActive = false;
        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        SetLevelCompleteActive(false);
        Input->HideCursor(true);

        std::cout << "LEVEL COMPLETE HIDDEN\n";
    }

private:
    void SetLevelCompleteActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(levelCompleteCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(LevelCompleteScript,
        levelCompleteCanvasGUID,
        originalCanvasPosition,
        hiddenPosition);
};

inline bool LevelCompleteScript::isLevelCompleteActive = false;
inline LevelCompleteScript* LevelCompleteScript::instance = nullptr;