#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"
#include "OptionsMenuScript.h"
#include "QuitMenuConfirmScript.h"
#include "QuitWindowConfirmScript.h"

/*
    UIButtonScript
    ==============
    A reusable button script for scene switching and quitting.

    Action types:
    0  = None
    1  = LoadScene
    2  = QuitGame
    3  = ResumeGame
    4  = Options
    5  = CloseOptions
    6  = MasterVolumeUp
    7  = MasterVolumeDown
    8  = BGMVolumeUp
    9  = BGMVolumeDown
    10 = SFXVolumeUp
    11 = SFXVolumeDown
    12 = MouseSensitivityUp
    13 = MouseSensitivityDown
    14 = QuitToMenuConfirm         (from Pause Menu)
    15 = CloseQuitMenuConfirm      (No / Back buttons on Quit Menu popup)
    16 = QuitToWindowConfirm       (from Pause Menu)
    17 = CloseQuitWindowConfirm    (No / Back buttons on Quit Window popup)
    18 = QuitToMenuConfirm_Lose    (from Lose Screen)
    19 = QuitToMenuConfirm_Win     (from Win Screen)
    20 = QuitToWindowConfirm_Lose  (from Lose Screen)
    21 = QuitToWindowConfirm_Win   (from Win Screen)
*/

class UIButtonScript : public TemplateSC {
public:

    enum class ButtonAction {
        None = 0,
        LoadScene = 1,
        QuitGame = 2,
        ResumeGame = 3,
        Options = 4,
        CloseOptions = 5,
        MasterVolumeUp = 6,
        MasterVolumeDown = 7,
        BGMVolumeUp = 8,
        BGMVolumeDown = 9,
        SFXVolumeUp = 10,
        SFXVolumeDown = 11,
        MouseSensitivityUp = 12,
        MouseSensitivityDown = 13,
        QuitToMenuConfirm = 14,
        CloseQuitMenuConfirm = 15,
        QuitToWindowConfirm = 16,
        CloseQuitWindowConfirm = 17,
        QuitToMenuConfirm_Lose = 18,
        QuitToMenuConfirm_Win = 19,
        QuitToWindowConfirm_Lose = 20,
        QuitToWindowConfirm_Win = 21,
        RestartScene = 22
    };

    int actionType = 0;

    utility::GUID currentSceneGUID;
    utility::GUID targetSceneGUID;

    utility::GUID selectSfxGUID;
    utility::GUID resumeSfxGUID;
    utility::GUID hoverSfxGUID;

    bool useKeyboardShortcuts = true;
    bool wasPressed = false;
    bool wasHovered = false;

    void Start() override {
        wasPressed = false;
        wasHovered = false;
        std::cout << "[UIButtonScript] Start() - Entity: " << entity << std::endl;
        std::cout << "  Action: " << GetActionName() << " (" << actionType << ")" << std::endl;
        std::cout << "  Current Scene: " << currentSceneGUID.GetToString() << std::endl;
        std::cout << "  Target Scene: " << targetSceneGUID.GetToString() << std::endl;
        std::cout << "  Keyboard shortcuts: " << (useKeyboardShortcuts ? "Enabled" : "Disabled") << std::endl;
    }

    void Update() override {
        // Handle sprite swapping
        if (ecsPtr->HasComponent<ecs::ButtonComponent>(entity) &&
            ecsPtr->HasComponent<ecs::SpriteComponent>(entity)) {

            auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);
            auto* spr = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);

            if (btn->isHovered && !wasHovered) {
                std::cout << "[UIButtonScript] Hover entered on entity: " << entity << "\n";

                if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
                    for (auto& af : ac->audioFiles) {
                        if (af.audioGUID == hoverSfxGUID && af.isSFX) {
                            af.requestPlay = true;
                            break;
                        }
                    }
                }
            }

            if (btn->useSpriteSwap) {
                if (btn->isPressed && !btn->pressedSprite.Empty()) {
                    spr->spriteGUID = btn->pressedSprite;
                }
                else if (btn->isHovered && !btn->hoveredSprite.Empty()) {
                    spr->spriteGUID = btn->hoveredSprite;
                }
                else if (!btn->normalSprite.Empty()) {
                    spr->spriteGUID = btn->normalSprite;
                }
            }

            wasHovered = btn->isHovered;
        }

        bool shouldTrigger = false;
        int triggeredAction = -1;

        // Check button click
        if (ecsPtr->HasComponent<ecs::ButtonComponent>(entity)) {
            auto* buttonComp = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);
            if (buttonComp->isPressed && !wasPressed) {
                shouldTrigger = true;
                triggeredAction = actionType;
            }
            wasPressed = buttonComp->isPressed;
        }

        // Check keyboard shortcuts
        if (useKeyboardShortcuts) {
            if (Input->IsKeyTriggered(keys::NUM0)) {
                shouldTrigger = true; triggeredAction = 0;
                std::cout << "[UIButtonScript] Key '0' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM1)) {
                shouldTrigger = true; triggeredAction = 1;
                std::cout << "[UIButtonScript] Key '1' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM2)) {
                shouldTrigger = true; triggeredAction = 2;
                std::cout << "[UIButtonScript] Key '2' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM4)) {
                shouldTrigger = true; triggeredAction = 4;
                std::cout << "[UIButtonScript] Key '4' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM5)) {
                shouldTrigger = true; triggeredAction = 5;
                std::cout << "[UIButtonScript] Key '5' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM6)) {
                shouldTrigger = true; triggeredAction = 6;
                std::cout << "[UIButtonScript] Key '6' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM7)) {
                shouldTrigger = true; triggeredAction = 7;
                std::cout << "[UIButtonScript] Key '7' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM8)) {
                shouldTrigger = true; triggeredAction = 16;
                std::cout << "[UIButtonScript] Key '8' pressed!\n";
            }
            else if (Input->IsKeyTriggered(keys::NUM9)) {
                shouldTrigger = true; triggeredAction = 22;
                std::cout << "[UIButtonScript] Key '9' pressed!\n";
            }
        }

        if (shouldTrigger && triggeredAction != -1) {
            std::cout << "[UIButtonScript] ===== TRIGGERED! =====\n";
            ExecuteAction(static_cast<ButtonAction>(triggeredAction));
        }
    }

private:
    void ExecuteAction(ButtonAction action) {
        if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
            for (auto& af : ac->audioFiles) {
                if (af.audioGUID == selectSfxGUID && af.isSFX) {
                    af.requestPlay = true;
                    break;
                }
            }
        }

        switch (action) {

        case ButtonAction::None:
            std::cout << "[UIButtonScript] Action: None\n";
            break;

        case ButtonAction::LoadScene:
            std::cout << "[UIButtonScript] Action: LoadScene\n";
            SwitchScene();
            break;

        case ButtonAction::QuitGame:
            std::cout << "[UIButtonScript] Action: QuitGame\n";
            Input->InputExitWindow();
            break;

        case ButtonAction::ResumeGame:
            std::cout << "[UIButtonScript] Action: ResumeGame\n";
            if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
                for (auto& af : ac->audioFiles) {
                    if (af.audioGUID == resumeSfxGUID && af.isSFX) {
                        af.requestPlay = true;
                        break;
                    }
                }
            }
            if (PauseMenuScript::instance)
                PauseMenuScript::instance->ResumeGame();
            break;

        case ButtonAction::Options:
            std::cout << "[UIButtonScript] Action: OPEN Options\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->OpenOptions();
            break;

        case ButtonAction::CloseOptions:
            std::cout << "[UIButtonScript] Action: CLOSE Options\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->CloseOptions();
            break;

        case ButtonAction::MasterVolumeUp:
            std::cout << "[UIButtonScript] Action: MasterVolumeUp\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->IncreaseMasterVolume();
            break;

        case ButtonAction::MasterVolumeDown:
            std::cout << "[UIButtonScript] Action: MasterVolumeDown\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->DecreaseMasterVolume();
            break;

        case ButtonAction::BGMVolumeUp:
            std::cout << "[UIButtonScript] Action: BGMVolumeUp\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->IncreaseBGMVolume();
            break;

        case ButtonAction::BGMVolumeDown:
            std::cout << "[UIButtonScript] Action: BGMVolumeDown\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->DecreaseBGMVolume();
            break;

        case ButtonAction::SFXVolumeUp:
            std::cout << "[UIButtonScript] Action: SFXVolumeUp\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->IncreaseSFXVolume();
            break;

        case ButtonAction::SFXVolumeDown:
            std::cout << "[UIButtonScript] Action: SFXVolumeDown\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->DecreaseSFXVolume();
            break;

        case ButtonAction::MouseSensitivityUp:
            std::cout << "[UIButtonScript] Action: MouseSensitivityUp\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->IncreaseMouseSensitivity();
            break;

        case ButtonAction::MouseSensitivityDown:
            std::cout << "[UIButtonScript] Action: MouseSensitivityDown\n";
            if (OptionsMenuScript::instance)
                OptionsMenuScript::instance->DecreaseMouseSensitivity();
            break;

            // ---------------------------------------------------------------
            // Quit to Menu confirm popup
            // ---------------------------------------------------------------
        case ButtonAction::QuitToMenuConfirm:
            std::cout << "[UIButtonScript] Action: QuitToMenuConfirm (from Pause)\n";
            if (QuitMenuConfirmScript::instance)
                QuitMenuConfirmScript::instance->Show(QuitMenuConfirmScript::CallerScreen::PauseMenu);
            break;

        case ButtonAction::QuitToMenuConfirm_Lose:
            std::cout << "[UIButtonScript] Action: QuitToMenuConfirm (from Lose)\n";
            if (QuitMenuConfirmScript::instance)
                QuitMenuConfirmScript::instance->Show(QuitMenuConfirmScript::CallerScreen::LoseScreen);
            break;

        case ButtonAction::QuitToMenuConfirm_Win:
            std::cout << "[UIButtonScript] Action: QuitToMenuConfirm (from Win)\n";
            if (QuitMenuConfirmScript::instance)
                QuitMenuConfirmScript::instance->Show(QuitMenuConfirmScript::CallerScreen::WinScreen);
            break;

        case ButtonAction::CloseQuitMenuConfirm:
            std::cout << "[UIButtonScript] Action: CloseQuitMenuConfirm\n";
            if (QuitMenuConfirmScript::instance)
                QuitMenuConfirmScript::instance->Hide();
            break;

            // ---------------------------------------------------------------
            // Quit to Window confirm popup
            // ---------------------------------------------------------------
        case ButtonAction::QuitToWindowConfirm:
            std::cout << "[UIButtonScript] Action: QuitToWindowConfirm (from Pause)\n";
            if (QuitWindowConfirmScript::instance)
                QuitWindowConfirmScript::instance->Show(QuitWindowConfirmScript::CallerScreen::PauseMenu);
            break;

        case ButtonAction::QuitToWindowConfirm_Lose:
            std::cout << "[UIButtonScript] Action: QuitToWindowConfirm (from Lose)\n";
            if (QuitWindowConfirmScript::instance)
                QuitWindowConfirmScript::instance->Show(QuitWindowConfirmScript::CallerScreen::LoseScreen);
            break;

        case ButtonAction::QuitToWindowConfirm_Win:
            std::cout << "[UIButtonScript] Action: QuitToWindowConfirm (from Win)\n";
            if (QuitWindowConfirmScript::instance)
                QuitWindowConfirmScript::instance->Show(QuitWindowConfirmScript::CallerScreen::WinScreen);
            break;

        case ButtonAction::CloseQuitWindowConfirm:
            std::cout << "[UIButtonScript] Action: CloseQuitWindowConfirm\n";
            if (QuitWindowConfirmScript::instance)
                QuitWindowConfirmScript::instance->Hide();
            break;

        case ButtonAction::RestartScene:
            std::cout << "[UIButtonScript] Action: RestartScene\n";
            Scenes->ReloadScene();
            break;

        default:
            std::cout << "[UIButtonScript] Unknown action: " << static_cast<int>(action) << "\n";
            break;
        }
    }

    void SwitchScene() {
        if (targetSceneGUID.Empty()) {
            std::cout << "[UIButtonScript] ERROR: targetSceneGUID is empty!\n";
            return;
        }
        std::cout << "[UIButtonScript] Switching to: " << targetSceneGUID.GetToString() << "\n";
        Scenes->ClearAllScene();
        Scenes->LoadScene(targetSceneGUID);
    }

    std::string GetActionName() {
        switch (static_cast<ButtonAction>(actionType)) {
        case ButtonAction::None:                     return "None";
        case ButtonAction::LoadScene:                return "LoadScene";
        case ButtonAction::QuitGame:                 return "QuitGame";
        case ButtonAction::ResumeGame:               return "ResumeGame";
        case ButtonAction::Options:                  return "Options";
        case ButtonAction::CloseOptions:             return "CloseOptions";
        case ButtonAction::MasterVolumeUp:           return "MasterVolumeUp";
        case ButtonAction::MasterVolumeDown:         return "MasterVolumeDown";
        case ButtonAction::BGMVolumeUp:              return "BGMVolumeUp";
        case ButtonAction::BGMVolumeDown:            return "BGMVolumeDown";
        case ButtonAction::SFXVolumeUp:              return "SFXVolumeUp";
        case ButtonAction::SFXVolumeDown:            return "SFXVolumeDown";
        case ButtonAction::MouseSensitivityUp:       return "MouseSensitivityUp";
        case ButtonAction::MouseSensitivityDown:     return "MouseSensitivityDown";
        case ButtonAction::QuitToMenuConfirm:        return "QuitToMenuConfirm";
        case ButtonAction::CloseQuitMenuConfirm:     return "CloseQuitMenuConfirm";
        case ButtonAction::QuitToWindowConfirm:      return "QuitToWindowConfirm";
        case ButtonAction::CloseQuitWindowConfirm:   return "CloseQuitWindowConfirm";
        case ButtonAction::QuitToMenuConfirm_Lose:   return "QuitToMenuConfirm_Lose";
        case ButtonAction::QuitToMenuConfirm_Win:    return "QuitToMenuConfirm_Win";
        case ButtonAction::QuitToWindowConfirm_Lose: return "QuitToWindowConfirm_Lose";
        case ButtonAction::QuitToWindowConfirm_Win:  return "QuitToWindowConfirm_Win";
        case ButtonAction::RestartScene:             return "RestartScene";
        default:                                     return "Unknown";
        }
    }

public:
    REFLECTABLE(UIButtonScript,
        actionType,
        currentSceneGUID,
        targetSceneGUID,
        useKeyboardShortcuts,
        wasPressed,
        resumeSfxGUID,
        selectSfxGUID,
        hoverSfxGUID);
};
