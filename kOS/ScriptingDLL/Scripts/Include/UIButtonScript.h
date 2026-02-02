#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

/*
    UIButtonScript
    ==============
    A reusable button script that attaches to any entity with a
    ButtonComponent and SpriteComponent.

    How to use:
    1. Attach this script to your button entity
    2. In the inspector, set "actionType" to one of the following:
        - 0: None (does nothing)
        - 1: LoadScene (switches to a different scene)
        - 2: QuitGame (exits the application)
    3. If you set actionType to 1 (LoadScene), also fill in:
        - "currentSceneGUID": drag the scene file you are currently IN
        - "targetSceneGUID": drag the scene file you want to go TO
*/

class UIButtonScript : public TemplateSC {
public:

    // What this button does when clicked (0=None, 1=LoadScene, 2=QuitGame)
    int actionType = 0;

    // The scene this button is currently in (will be cleared on click)
    utility::GUID currentSceneGUID;

    // The scene to load when clicked
    utility::GUID targetSceneGUID;

    // Tracks whether the button was pressed last frame
    bool wasPressed = false;

    void Start() override {
        wasPressed = false;

        std::cout << "[UIButtonScript] Start() called on entity: " << entity << std::endl;
        std::cout << "  actionType: " << actionType << std::endl;
        std::cout << "  currentSceneGUID: " << currentSceneGUID.GetToString() << std::endl;
        std::cout << "  targetSceneGUID: " << targetSceneGUID.GetToString() << std::endl;
        std::cout << "  Has ButtonComponent: " << ecsPtr->HasComponent<ecs::ButtonComponent>(entity) << std::endl;
    }

    void Update() override {
        if (!ecsPtr->HasComponent<ecs::ButtonComponent>(entity)) {
            return;
        }

        auto* buttonComp = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);

        // Detect the exact frame the button is first clicked
        if (buttonComp->isPressed && !wasPressed) {
            std::cout << "[UIButtonScript] Button CLICKED on entity: " << entity << std::endl;
            ExecuteAction();
        }

        wasPressed = buttonComp->isPressed;
    }

private:

    void ExecuteAction() {
        switch (actionType) {

        case 0: // None
            std::cout << "[UIButtonScript] actionType is None - nothing to do." << std::endl;
            break;

        case 1: // LoadScene
            SwitchScene();
            break;

        case 2: // QuitGame
            std::cout << "[UIButtonScript] Quitting game..." << std::endl;
            Input->InputExitWindow();
            break;

        default:
            std::cout << "[UIButtonScript] Unknown actionType: " << actionType << std::endl;
            break;
        }
    }

    void SwitchScene() {
        if (currentSceneGUID.Empty() || targetSceneGUID.Empty()) {
            std::cout << "[UIButtonScript] ERROR: currentSceneGUID or targetSceneGUID is empty!" << std::endl;
            return;
        }

        std::cout << "[UIButtonScript] Clearing scene: " << currentSceneGUID.GetToString() << std::endl;
        std::cout << "[UIButtonScript] Loading scene: " << targetSceneGUID.GetToString() << std::endl;

        // Step 1: Clear the current scene
        Scenes->ClearScene(currentSceneGUID);

        // Step 2: Load the target scene
        Scenes->LoadScene(targetSceneGUID);
    }

public:
    REFLECTABLE(UIButtonScript,
        actionType,
        currentSceneGUID,
        targetSceneGUID,
        wasPressed);
};