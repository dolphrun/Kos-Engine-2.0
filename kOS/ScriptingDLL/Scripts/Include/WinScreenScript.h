#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include <iostream>

class WinScreenScript : public TemplateSC {
public:
    // Static flag to indicate if win screen is active
    static bool isWinScreenActive;

    utility::GUID winScreenCanvasGUID;
    ecs::EntityID winScreenCanvasID = 0;

    utility::GUID currentScoreTextGUID;
    utility::GUID bestScoreTextGUID;

    ecs::EntityID currentScoreTextID = 0;
    ecs::EntityID bestScoreTextID = 0;

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    bool hasShownWinScreen = false;

    void Start() override {
        isWinScreenActive = false; // reset static flag each run
        winScreenCanvasID = ecsPtr->GetEntityIDFromGUID(winScreenCanvasGUID);
        currentScoreTextID = ecsPtr->GetEntityIDFromGUID(currentScoreTextGUID);
        bestScoreTextID = ecsPtr->GetEntityIDFromGUID(bestScoreTextGUID);

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(winScreenCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }

        ecsPtr->SetTimeScale(1.0f);
        // Always hide on start
        SetWinScreenActive(false);
    }

    void Update() override {
        // Nothing needed here for now
    }

    // Call this when the player wins
    void ShowWinScreen() {
        if (hasShownWinScreen)
            return; // prevents double activation

        hasShownWinScreen = true;
        isWinScreenActive = true; // block other inputs if needed
        ecsPtr->SetTimeScale(0.0f);
        SetWinScreenActive(true);
        Input->HideCursor(false);

        if (auto* currentText = ecsPtr->GetComponent<ecs::TextComponent>(currentScoreTextID)) {
            currentText->text = std::to_string(ScoreManagerScript::lastScore);
        }
        if (auto* bestText = ecsPtr->GetComponent<ecs::TextComponent>(bestScoreTextID)) {
            bestText->text = std::to_string(ScoreManagerScript::bestScore);
        }

        std::cout << "WIN SCREEN SHOWN\n";
    }

    // Optional: hide the win screen if needed
    void HideWinScreen() {
        if (!hasShownWinScreen)
            return;

        hasShownWinScreen = false;
        isWinScreenActive = false;
        ecsPtr->SetTimeScale(1.0f);
        SetWinScreenActive(false);
        Input->HideCursor(true);

        std::cout << "WIN SCREEN HIDDEN\n";
    }

private:
    void SetWinScreenActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(winScreenCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(WinScreenScript, winScreenCanvasGUID, hiddenPosition, originalCanvasPosition, currentScoreTextGUID, bestScoreTextGUID);
};

// Static definition
bool WinScreenScript::isWinScreenActive = false;
