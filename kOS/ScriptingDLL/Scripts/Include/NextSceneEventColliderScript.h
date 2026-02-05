#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

class NextSceneColliderScript : public TemplateSC {
public:
    // The scene to load when player enters this trigger
    utility::GUID nextSceneGUID;

    // Minimum time before trigger can activate (prevents instant triggering on level start)
    float minActivationTime = 2.0f;

    // Internal timer
    float timer = 0.0f;

    // Track if already triggered (prevent multiple triggers)
    bool hasTriggered = false;

    void Start() {
        timer = 0.0f;
        hasTriggered = false;

        std::cout << "[NextSceneColliderScript] Start()" << std::endl;
        std::cout << "  Next Scene GUID: " << nextSceneGUID.GetToString() << std::endl;
        std::cout << "  Min Activation Time: " << minActivationTime << "s" << std::endl;

        // Set up trigger callback
        physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
            // Check if it's the player and enough time has passed
            if (ecsPtr->GetComponent<ecs::NameComponent>(col.otherEntityID)->entityTag == "Player"
                && timer >= minActivationTime
                && !hasTriggered) {

                std::cout << "\n=== PLAYER REACHED END OF LEVEL ===" << std::endl;
                std::cout << "Timer: " << timer << "s (min: " << minActivationTime << "s)" << std::endl;

                TransitionToNextScene();
            }
            });
    }

    void Update() {
        timer += ecsPtr->m_GetDeltaTime();
    }

private:
    void TransitionToNextScene() {
        if (nextSceneGUID.Empty()) {
            std::cout << "[NextSceneColliderScript] ERROR: nextSceneGUID is empty! Cannot load next scene." << std::endl;
            return;
        }

        hasTriggered = true;  // Prevent multiple triggers

        std::cout << "[NextSceneColliderScript] === TRANSITIONING TO NEXT SCENE ===" << std::endl;
        std::cout << "  Clearing all scenes..." << std::endl;
        std::cout << "  Loading: " << nextSceneGUID.GetToString() << std::endl;

        // Clear all current scenes (same as UIButtonScript)
        Scenes->ClearAllScene(false);  // false = keep prefabs

        // Load next scene
        Scenes->LoadScene(nextSceneGUID);

        std::cout << "[NextSceneColliderScript] Scene transition queued!" << std::endl;
    }

public:
    REFLECTABLE(NextSceneColliderScript, nextSceneGUID, minActivationTime, timer, hasTriggered);
};