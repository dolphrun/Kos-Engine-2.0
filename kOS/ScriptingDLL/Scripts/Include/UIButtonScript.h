#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"

/*
    UIButtonScript
    ==============
    A reusable button script for scene switching and quitting.

    Action types:
    0 = None
    1 = LoadScene
    2 = QuitGame
    3 = ResumeGame
    4 = Options (stub)
*/

class UIButtonScript : public TemplateSC {
public:

    enum class ButtonAction {
        None = 0,
        LoadScene = 1,
        QuitGame = 2,
        ResumeGame = 3,
        Options = 4
    };

    int actionType = 0;

    utility::GUID currentSceneGUID;
    utility::GUID targetSceneGUID;

    utility::GUID selectSfxGUID;
    utility::GUID resumeSfxGUID;
    utility::GUID hoverSfxGUID;

    bool useKeyboardShortcuts = true;
    bool wasPressed = false;

    void Start() override {
        wasPressed = false;

        std::cout << "[UIButtonScript] Start() - Entity: " << entity << std::endl;
        std::cout << "  Action: " << GetActionName() << " (" << actionType << ")" << std::endl;
        std::cout << "  Current Scene: " << currentSceneGUID.GetToString() << std::endl;
        std::cout << "  Target Scene: " << targetSceneGUID.GetToString() << std::endl;
        std::cout << "  Keyboard shortcuts: " << (useKeyboardShortcuts ? "Enabled (0/1/2)" : "Disabled") << std::endl;
    }

    void Update() override {
        // Handle sprite swapping
        if (ecsPtr->HasComponent<ecs::ButtonComponent>(entity) &&
            ecsPtr->HasComponent<ecs::SpriteComponent>(entity)) {

            auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);
            auto* spr = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);

            if (btn->useSpriteSwap) {
                if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
                    for (auto& af : ac->audioFiles) {
                        if (af.audioGUID == hoverSfxGUID && af.isSFX) {
                            af.requestPlay = true;
                            break;
                        }
                    }
                }

                if (btn->isPressed && !btn->pressedSprite.Empty())
                    spr->spriteGUID = btn->pressedSprite;
                else if (btn->isHovered && !btn->hoveredSprite.Empty()) {

 
                    spr->spriteGUID = btn->hoveredSprite;

                }
                else if (!btn->normalSprite.Empty())
                    spr->spriteGUID = btn->normalSprite;

                if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
                    for (auto& af : ac->audioFiles) {
                        if (af.audioGUID == hoverSfxGUID && af.isSFX) {
                            af.requestPlay = true;
                            break;
                        }
                    }
                }
                spr->spriteGUID = btn->hoveredSprite;
            }
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
            std::cout << "[UIButtonScript] Action: Options (stub)\n";
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
        case ButtonAction::None:       return "None";
        case ButtonAction::LoadScene:  return "LoadScene";
        case ButtonAction::QuitGame:   return "QuitGame";
        case ButtonAction::ResumeGame: return "ResumeGame";
        case ButtonAction::Options:    return "Options";
        default:                       return "Unknown";
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