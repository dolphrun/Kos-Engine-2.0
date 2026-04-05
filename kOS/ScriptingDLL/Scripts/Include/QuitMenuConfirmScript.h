#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"
#include "LoseScreenScript.h"
#include "WinScreenScript.h"

/*
    QuitMenuConfirmScript
    =====================
    Attach this to the Quit Menu Canvas GameObject.

    Inspector setup:
      - quitMenuCanvasGUID  -> the Quit Menu Canvas itself
      - No Button           -> UIButtonScript action: CloseQuitMenuConfirm      (15)
      - Back Button         -> UIButtonScript action: CloseQuitMenuConfirm      (15)
      - Yes Button          -> UIButtonScript action: LoadScene (already configured)

    Caller actions (set on the button that OPENS this popup):
      - From Pause Menu     -> UIButtonScript action: QuitToMenuConfirm         (14)
      - From Lose Screen    -> UIButtonScript action: QuitToMenuConfirm_Lose    (18)
      - From Win Screen     -> UIButtonScript action: QuitToMenuConfirm_Win     (19)
*/

class QuitMenuConfirmScript : public TemplateSC {
public:
    static QuitMenuConfirmScript* instance;
    static bool isQuitMenuConfirmActive;

    // Tracks which screen opened this popup so Hide() restores the correct one
    enum class CallerScreen { None = 0, PauseMenu, LoseScreen, WinScreen };
    CallerScreen callerScreen = CallerScreen::None;

    utility::GUID quitMenuCanvasGUID;
    ecs::EntityID quitMenuCanvasID = 0;

    glm::vec3 originalPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    void Start() override {
        instance = this;
        isQuitMenuConfirmActive = false;
        callerScreen = CallerScreen::None;

        quitMenuCanvasID = ecsPtr->GetEntityIDFromGUID(quitMenuCanvasGUID);

        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(quitMenuCanvasID)) {
            originalPosition = t->LocalTransformation.position;
        }

        SetActive(false);
        std::cout << "[QuitMenuConfirmScript] Start() complete.\n";
    }

    void Update() override {}

    // Called by UIButtonScript actions 14, 18, 19
    // Hides the caller screen and shows this confirm popup
    void Show(CallerScreen caller = CallerScreen::PauseMenu) {
        isQuitMenuConfirmActive = true;
        callerScreen = caller;

        // Hide whichever screen opened this popup
        if (caller == CallerScreen::PauseMenu && PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(false);
        else if (caller == CallerScreen::LoseScreen && LoseScreenScript::instance)
            LoseScreenScript::instance->SetLoseScreenActive(false);
        else if (caller == CallerScreen::WinScreen && WinScreenScript::instance)
            WinScreenScript::instance->SetWinScreenActive(false);

        SetActive(true);
        std::cout << "[QuitMenuConfirmScript] Shown (caller=" << static_cast<int>(caller) << ").\n";
    }

    // Called by UIButtonScript action 15 (No / Back buttons)
    // Dismisses popup and restores the screen that opened it
    void Hide() {
        isQuitMenuConfirmActive = false;
        SetActive(false);

        // Restore whichever screen opened this popup
        if (callerScreen == CallerScreen::PauseMenu && PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(true);
        else if (callerScreen == CallerScreen::LoseScreen && LoseScreenScript::instance)
            LoseScreenScript::instance->SetLoseScreenActive(true);
        else if (callerScreen == CallerScreen::WinScreen && WinScreenScript::instance)
            WinScreenScript::instance->SetWinScreenActive(true);

        callerScreen = CallerScreen::None;
        std::cout << "[QuitMenuConfirmScript] Hidden, caller screen restored.\n";
    }

    void SetActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(quitMenuCanvasID)) {
            t->LocalTransformation.position = active ? originalPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(QuitMenuConfirmScript,
        quitMenuCanvasGUID, quitMenuCanvasID,
        originalPosition, hiddenPosition);
};

inline QuitMenuConfirmScript* QuitMenuConfirmScript::instance = nullptr;
inline bool                   QuitMenuConfirmScript::isQuitMenuConfirmActive = false;
