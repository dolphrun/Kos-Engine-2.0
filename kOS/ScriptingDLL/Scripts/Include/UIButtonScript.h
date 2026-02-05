#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

/*
    UIButtonScript
    ==============
    A reusable button script for scene switching and quitting.

    Features:
    - Click button OR press number keys (0/1/2) to trigger actions
    - Dropdown for action type

    Keyboard shortcuts:
    - Press 0: Does nothing (None action)
    - Press 1: Load scene (if currentSceneGUID and targetSceneGUID are set)
    - Press 2: Quit game

    How to use:
    1. Attach to button entity with ButtonComponent + SpriteComponent
    2. Select actionType from dropdown (0=None, 1=LoadScene, 2=QuitGame)
    3. For LoadScene: drag scene files to currentSceneGUID and targetSceneGUID
    4. Can trigger by clicking button OR pressing 0/1/2 keys
*/

class UIButtonScript : public TemplateSC {
public:

    // ===== Action Type Enum =====
    // NOTE: To make this show as dropdown in inspector, you may need to tell
    // your reflection system to treat this as an enum. If it's showing as a
    // slider/scroller instead of dropdown, ask your engine programmer how to
    // register enums for the reflection system.
    enum class ButtonAction {
        None = 0,
        LoadScene = 1,
        QuitGame = 2
    };

    // Action selector (should show as dropdown if reflection supports it)
    // 0 = None, 1 = LoadScene, 2 = QuitGame
    int actionType = 0;

    // Scene switching (only used if actionType == 1)
    utility::GUID currentSceneGUID;  // Scene to clear (can leave empty, ClearAllScene clears everything)
    utility::GUID targetSceneGUID;   // Scene to load

    // Keyboard shortcuts toggle
    bool useKeyboardShortcuts = true;  // Enable number key shortcuts (0/1/2)

    // Internal state
    bool wasPressed = false;

    void Start() override {
        wasPressed = false;

        std::cout << "[UIButtonScript] Start() - Entity: " << entity << std::endl;
        std::cout << "  Action: " << GetActionName() << std::endl;
        std::cout << "  Current Scene: " << currentSceneGUID.GetToString() << std::endl;
        std::cout << "  Target Scene: " << targetSceneGUID.GetToString() << std::endl;
        std::cout << "  Keyboard shortcuts: " << (useKeyboardShortcuts ? "Enabled (0/1/2)" : "Disabled") << std::endl;
    }

    void Update() override {
        bool shouldTrigger = false;
        int triggeredAction = -1;  // -1 = no trigger, 0/1/2 = specific action

        // Check button press (uses the assigned actionType)
        if (ecsPtr->HasComponent<ecs::ButtonComponent>(entity)) {
            auto* buttonComp = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);

            if (buttonComp->isPressed && !wasPressed) {
                shouldTrigger = true;
                triggeredAction = actionType;  // Use the button's assigned action
            }

            wasPressed = buttonComp->isPressed;
        }

        // Check keyboard shortcuts (each number key triggers its corresponding action)
        if (useKeyboardShortcuts) {
            if (Input->IsKeyTriggered(keys::NUM0)) {
                shouldTrigger = true;
                triggeredAction = 0;  // None action
                std::cout << "[UIButtonScript] Key '0' pressed!" << std::endl;
            }
            else if (Input->IsKeyTriggered(keys::NUM1)) {
                shouldTrigger = true;
                triggeredAction = 1;  // LoadScene action
                std::cout << "[UIButtonScript] Key '1' pressed!" << std::endl;
            }
            else if (Input->IsKeyTriggered(keys::NUM2)) {
                shouldTrigger = true;
                triggeredAction = 2;  // QuitGame action
                std::cout << "[UIButtonScript] Key '2' pressed!" << std::endl;
            }
        }

        // Execute the triggered action
        if (shouldTrigger && triggeredAction != -1) {
            std::cout << "[UIButtonScript] ===== TRIGGERED! =====" << std::endl;
            ExecuteAction(static_cast<ButtonAction>(triggeredAction));
        }
    }

private:

    void ExecuteAction(ButtonAction action) {
        switch (action) {

        case ButtonAction::None:
            std::cout << "[UIButtonScript] Action 0: None - doing nothing." << std::endl;
            break;

        case ButtonAction::LoadScene:
            std::cout << "[UIButtonScript] Action 1: LoadScene" << std::endl;
            SwitchScene();
            break;

        case ButtonAction::QuitGame:
            std::cout << "[UIButtonScript] Action 2: QuitGame" << std::endl;
            Input->InputExitWindow();
            break;

        default:
            std::cout << "[UIButtonScript] Unknown action: " << static_cast<int>(action) << std::endl;
            break;
        }
    }

    void SwitchScene() {
        if (targetSceneGUID.Empty()) {
            std::cout << "[UIButtonScript] ERROR: targetSceneGUID is empty! Can't load scene." << std::endl;
            std::cout << "  Target: " << targetSceneGUID.GetToString() << std::endl;
            return;
        }

        std::cout << "[UIButtonScript] === SWITCHING SCENES ===" << std::endl;
        std::cout << "  Clearing all scenes..." << std::endl;
        std::cout << "  Loading: " << targetSceneGUID.GetToString() << std::endl;

        // Clear all scenes (like CutSceneScript does)
        Scenes->ClearAllScene();

        // Load target scene
        Scenes->LoadScene(targetSceneGUID);

        std::cout << "[UIButtonScript] Scene switch complete!" << std::endl;
    }

    // Helper to get action name for debug output
    std::string GetActionName() {
        switch (static_cast<ButtonAction>(actionType)) {
        case ButtonAction::None: return "None (0)";
        case ButtonAction::LoadScene: return "LoadScene (1)";
        case ButtonAction::QuitGame: return "QuitGame (2)";
        default: return "Unknown";
        }
    }

public:
    REFLECTABLE(UIButtonScript,
        actionType,
        currentSceneGUID,
        targetSceneGUID,
        useKeyboardShortcuts,
        wasPressed);
};