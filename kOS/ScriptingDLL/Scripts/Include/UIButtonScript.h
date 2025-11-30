#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

class UIButtonScript : public TemplateSC {
public:

    // ===== Action Types =====
    enum class ButtonAction {
        None = 0,
        LoadScene = 1,
        QuitGame = 2,
        CustomCallback = 3
    };

    // Button action configuration
    int actionType = 0;  // 0=None, 1=LoadScene, 2=QuitGame, 3=Custom
    std::string sceneToLoad = ""; 
    bool wasPressed = false;  


    void Start() override {
        wasPressed = false;

        auto* buttonComp = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);
        auto* sprite = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);

        std::cout << "Action Type: " << actionType << std::endl;
        //std::cout << "Scene to Load: " << (sceneToLoad.empty() ? "(none)" : sceneToLoad) << std::endl;
    }

    void Update() override {
        // Get the ButtonComponent to check if it's being clicked
        auto* buttonComp = ecsPtr->GetComponent<ecs::ButtonComponent>(entity);

        // Check if button was just pressed (transition from not pressed to pressed)
        if (buttonComp->isPressed && !wasPressed) {
            ExecuteButtonAction();
        }

        // Update state for next frame
        wasPressed = buttonComp->isPressed;
    }

private:

    void ExecuteButtonAction() {
        ButtonAction action = static_cast<ButtonAction>(actionType);

        switch (action) {
        case ButtonAction::None:
            //std::cout << "Button has no action assigned" << std::endl;
            break;

        case ButtonAction::LoadScene:
            if (!sceneToLoad.empty()) {
                std::cout << "Loading scene: " << sceneToLoad << std::endl;
                Scenes->LoadScene(sceneToLoad);
            }
            else {
                std::cout << "WARNING: LoadScene selected but no scene specified!" << std::endl;
            }
            break;

        case ButtonAction::QuitGame:
            std::cout << "Quitting game..." << std::endl;
			// IDK HOW TO QUIT THE GAME FROM HERE
            //glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
            break;

        case ButtonAction::CustomCallback:
            std::cout << "Custom callback triggered!" << std::endl;
            // Add your custom code here
            // Example: Play sound, toggle pause menu, etc.
            break;
        }
    }

public:
    REFLECTABLE(UIButtonScript,
        actionType,
        sceneToLoad,
        wasPressed);
};