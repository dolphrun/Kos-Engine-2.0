#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"
#include "LoseScreenScript.h"
#include "WinScreenScript.h"

/*
    QuitWindowConfirmScript
    =======================
    Attach this to the Quit Window Canvas GameObject.

    Inspector setup:
      - quitWindowCanvasGUID -> the Quit Window Canvas itself
      - No Button            -> UIButtonScript action: CloseQuitWindowConfirm      (17)
      - Back Button          -> UIButtonScript action: CloseQuitWindowConfirm      (17)
      - Yes Button           -> UIButtonScript action: QuitGame (2) (already configured)

    Caller actions (set on the button that OPENS this popup):
      - From Pause Menu      -> UIButtonScript action: QuitToWindowConfirm         (16)
      - From Lose Screen     -> UIButtonScript action: QuitToWindowConfirm_Lose    (20)
      - From Win Screen      -> UIButtonScript action: QuitToWindowConfirm_Win     (21)
*/

class QuitWindowConfirmScript : public TemplateSC {
public:
    static QuitWindowConfirmScript* instance;
    static bool isQuitWindowConfirmActive;

    // Tracks which screen opened this popup so Hide() restores the correct one
    enum class CallerScreen { None = 0, PauseMenu, LoseScreen, WinScreen };
    CallerScreen callerScreen = CallerScreen::None;

    utility::GUID quitWindowCanvasGUID;
    ecs::EntityID quitWindowCanvasID = 0;

    glm::vec3 originalPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    void Start() override {
        instance = this;
        isQuitWindowConfirmActive = false;
        callerScreen = CallerScreen::None;

        quitWindowCanvasID = ecsPtr->GetEntityIDFromGUID(quitWindowCanvasGUID);

        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(quitWindowCanvasID)) {
            originalPosition = t->LocalTransformation.position;
        }

        SetActive(false);
        std::cout << "[QuitWindowConfirmScript] Start() complete.\n";
    }

    void Update() override {}

    // Called by UIButtonScript actions 16, 20, 21
    // Hides the caller screen and shows this confirm popup
    void Show(CallerScreen caller = CallerScreen::PauseMenu) {
        isQuitWindowConfirmActive = true;
        callerScreen = caller;

        // Hide whichever screen opened this popup
        if (caller == CallerScreen::PauseMenu && PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(false);
        else if (caller == CallerScreen::LoseScreen && LoseScreenScript::instance)
            LoseScreenScript::instance->SetLoseScreenActive(false);
        else if (caller == CallerScreen::WinScreen && WinScreenScript::instance)
            WinScreenScript::instance->SetWinScreenActive(false);

        SetActive(true);
        std::cout << "[QuitWindowConfirmScript] Shown (caller=" << static_cast<int>(caller) << ").\n";
    }

    // Called by UIButtonScript action 17 (No / Back buttons)
    // Dismisses popup and restores the screen that opened it
    void Hide() {
        isQuitWindowConfirmActive = false;
        SetActive(false);

        // Restore whichever screen opened this popup
        if (callerScreen == CallerScreen::PauseMenu && PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(true);
        else if (callerScreen == CallerScreen::LoseScreen && LoseScreenScript::instance)
            LoseScreenScript::instance->SetLoseScreenActive(true);
        else if (callerScreen == CallerScreen::WinScreen && WinScreenScript::instance)
            WinScreenScript::instance->SetWinScreenActive(true);

        callerScreen = CallerScreen::None;
        std::cout << "[QuitWindowConfirmScript] Hidden, caller screen restored.\n";
    }

    void SetActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(quitWindowCanvasID)) {
            t->LocalTransformation.position = active ? originalPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(QuitWindowConfirmScript,
        quitWindowCanvasGUID, quitWindowCanvasID,
        originalPosition, hiddenPosition);
};

inline QuitWindowConfirmScript* QuitWindowConfirmScript::instance = nullptr;
inline bool                     QuitWindowConfirmScript::isQuitWindowConfirmActive = false;
