#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PlayerManagerScript.h"

class ManaUIScript : public TemplateSC {
public:
    // Reference to the player object to read mana from
    utility::GUID playerObject;
    ecs::EntityID playerObjectID;

    // Mana bar sprite
    utility::GUID manabarSprite;

    void Start() override {
        // Resolve the player entity ID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Initialize with current mana
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            float manaPercentage = (playerMgr->currMana / playerMgr->maxMana) * 100.0f;
            UpdateManaBarSprite(manaPercentage);
        }
    }

    void Update() override {
        // Read mana from PlayerManagerScript
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            // Calculate mana percentage
            float manaPercentage = (playerMgr->currMana / playerMgr->maxMana) * 100.0f;

            // Clamp between 0 and 100
            manaPercentage = std::clamp(manaPercentage, 0.0f, 100.0f);

            // Update the sprite based on mana percentage
            UpdateManaBarSprite(manaPercentage);
        }
    }

private:
    glm::vec3 originalScale = glm::vec3(1.0f);      // Store original scale
    glm::vec3 originalPosition = glm::vec3(0.0f);   // Store original position
    bool initialized = false;

    void UpdateManaBarSprite(float manaPercentage) {
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity)) {
            // Set the sprite
            sc->spriteGUID = manabarSprite;

            // Enable custom UV cropping
            sc->useCustomUV = true;

            // Calculate UV coordinates to crop the sprite horizontally
            // The sprite will be cropped from right to left based on mana percentage
            float uvWidth = manaPercentage / 100.0f;

            // Set UV coordinates - crop from the right
            sc->uvMin = glm::vec2(0.0f, 0.0f);           // Bottom-left (fixed)
            sc->uvMax = glm::vec2(uvWidth, 1.0f);        // Top-right (X adjusts with mana)
        }

        // Get the transform component to adjust scale and position
        if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
            // Store the original scale and position on first run
            if (!initialized) {
                originalScale = tc->LocalTransformation.scale;
                originalPosition = tc->LocalTransformation.position;
                initialized = true;
            }

            // Calculate the scale factor based on mana percentage
            float scaleFactor = manaPercentage / 100.0f;

            // Scale the sprite horizontally (X-axis) to match the UV crop
            tc->LocalTransformation.scale.x = originalScale.x * scaleFactor;
            tc->LocalTransformation.scale.y = originalScale.y;
            tc->LocalTransformation.scale.z = originalScale.z;

            // Adjust position to keep the LEFT edge anchored
            // When scaling from center, we need to shift left by half the reduction
            float scaleReduction = originalScale.x * (1.0f - scaleFactor);
            tc->LocalTransformation.position.x = originalPosition.x - (scaleReduction * 0.5f);
            tc->LocalTransformation.position.y = originalPosition.y;
            tc->LocalTransformation.position.z = originalPosition.z;
        }
    }

public:
    REFLECTABLE(ManaUIScript, playerObject, playerObjectID, manabarSprite,
        originalScale, originalPosition, initialized);
};