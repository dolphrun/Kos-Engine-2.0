#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include <iostream>
#include "ScoreManagerScript.h"

class LoseScreenScript : public TemplateSC {
public:
    // Static flag to indicate if lose screen is active
    static bool isLoseScreenActive;

    utility::GUID loseScreenCanvasGUID;
    ecs::EntityID loseScreenCanvasID = 0;

    utility::GUID currentScoreTextGUID;
    utility::GUID bestScoreTextGUID;

    ecs::EntityID currentScoreTextID = 0;
    ecs::EntityID bestScoreTextID = 0;

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    bool hasShownLoseScreen = false;

    void Start() override{
		isLoseScreenActive = false; // reset static flag each run 
        loseScreenCanvasID = ecsPtr->GetEntityIDFromGUID(loseScreenCanvasGUID);
        currentScoreTextID = ecsPtr->GetEntityIDFromGUID(currentScoreTextGUID);
        bestScoreTextID = ecsPtr->GetEntityIDFromGUID(bestScoreTextGUID);

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(loseScreenCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }

        ecsPtr->SetTimeScale(1.0f);
        // Always hide on start
        SetLoseScreenActive(false);
    }

    void Update() override {
        // Nothing needed here.
    }

    // Call this when the player dies
    void ShowLoseScreen()
    {
        if (hasShownLoseScreen)
            return; // prevents double activation

        hasShownLoseScreen = true;
        isLoseScreenActive = true; // BLOCK pause menu
        ecsPtr->SetTimeScale(0.0f);
        SetLoseScreenActive(true);
        Input->HideCursor(false);

        if (auto* currentText = ecsPtr->GetComponent<ecs::TextComponent>(currentScoreTextID)) {
            currentText->text = std::to_string(ScoreManagerScript::lastScore);
        }
        if (auto* bestText = ecsPtr->GetComponent<ecs::TextComponent>(bestScoreTextID)) {
            bestText->text = std::to_string(ScoreManagerScript::bestScore);
        }

        std::cout << "LOSE SCREEN SHOWN\n";
    }

    // Optional: hide the lose screen if needed
    void HideLoseScreen()
    {
        if (!hasShownLoseScreen)
            return;

        hasShownLoseScreen = false;
        isLoseScreenActive = false; // unblock pause menu
        ecsPtr->SetTimeScale(1.0f);
        SetLoseScreenActive(false);
        Input->HideCursor(true);

        std::cout << "LOSE SCREEN HIDDEN\n";
    }

private:
    void SetLoseScreenActive(bool active)
    {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(loseScreenCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(LoseScreenScript, loseScreenCanvasGUID, hiddenPosition, originalCanvasPosition, currentScoreTextGUID, bestScoreTextGUID);
};

// Static definition
bool LoseScreenScript::isLoseScreenActive = false;
