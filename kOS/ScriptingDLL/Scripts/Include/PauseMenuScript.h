#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "LoseScreenScript.h"
#include "WinScreenScript.h"

class PauseMenuScript : public TemplateSC {
public:
    static bool requestRestart;
    static bool isPaused;
    static PauseMenuScript* instance;

    utility::GUID pauseMenuCanvasGUID;
    ecs::EntityID pauseMenuCanvasID = 0;

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    void Start() override {
        instance = this;
        isPaused = false;
        pauseMenuCanvasID = ecsPtr->GetEntityIDFromGUID(pauseMenuCanvasGUID);

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(pauseMenuCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }
        SetPauseMenuActive(false);
    }

    void Update() override {
        // ESC is handled by PlayerManagerScript
    }

    void TogglePause() {
        if (LoseScreenScript::isLoseScreenActive || WinScreenScript::isWinScreenActive) return;
        isPaused ? ResumeGame() : PauseGame();
    }

    void PauseGame() {
        isPaused = true;
        ecsPtr->SetTimeScale(0.0f);
        ecsPtr->SetState(WAIT);
        SetPauseMenuActive(true);
        Input->HideCursor(false);
        std::cout << "Game Paused\n";
    }

    void ResumeGame() {
        isPaused = false;
        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        SetPauseMenuActive(false);
        Input->HideCursor(true);
        std::cout << "Game Resumed\n";
    }

    void RestartGame() {
        requestRestart = true;
        isPaused = false;
        SetPauseMenuActive(false);
    }

    bool IsPaused() const { return isPaused; }

private:
    void SetPauseMenuActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(pauseMenuCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(PauseMenuScript, pauseMenuCanvasGUID, originalCanvasPosition, hiddenPosition);
};

inline bool PauseMenuScript::requestRestart = false;
inline bool PauseMenuScript::isPaused = false;
inline PauseMenuScript* PauseMenuScript::instance = nullptr;