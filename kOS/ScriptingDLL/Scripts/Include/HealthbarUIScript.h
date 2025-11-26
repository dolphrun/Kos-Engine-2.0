#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

class HealthBarScript : public TemplateSC {
public:
    int healthValue = 6;  // Current health (synced with PlayerManagerScript)
    int lastHealthValue = -1; // Used to check if health changed

    utility::GUID playerObject;  // Reference to the player object
    ecs::EntityID playerObjectID;

    // References to all 6 health bar sprites
    utility::GUID healthbar0Sprite;
    utility::GUID healthbar1Sprite;
    utility::GUID healthbar2Sprite;
    utility::GUID healthbar3Sprite;
    utility::GUID healthbar4Sprite;
    utility::GUID healthbar5Sprite;
    utility::GUID healthbar6Sprite;

    void Start() override {
        // Get the entity ID from the player object GUID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Initialize the sprite on start
        UpdateHealthBarSprite();
    }

    void Update() override {
        // Fetch the health from PlayerManagerScript
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            healthValue = playerMgr->currPlayerHitPoints;
        }

        // Only update the sprite if health has changed
        if (healthValue != lastHealthValue) {
            UpdateHealthBarSprite();
            lastHealthValue = healthValue;
        }
    }

private:
    void UpdateHealthBarSprite() {
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity)) {
            if (healthValue >= 6) sc->spriteGUID = healthbar6Sprite;
            else if (healthValue >= 5) sc->spriteGUID = healthbar5Sprite;
            else if (healthValue >= 4) sc->spriteGUID = healthbar4Sprite;
            else if (healthValue >= 3) sc->spriteGUID = healthbar3Sprite;
            else if (healthValue >= 2) sc->spriteGUID = healthbar2Sprite;
            else if (healthValue >= 1) sc->spriteGUID = healthbar1Sprite;
            else sc->spriteGUID = healthbar0Sprite;
        }
    }

public:
    REFLECTABLE(HealthBarScript, healthValue, lastHealthValue, playerObject, playerObjectID, healthbar0Sprite,
        healthbar1Sprite, healthbar2Sprite, healthbar3Sprite,
        healthbar4Sprite, healthbar5Sprite, healthbar6Sprite);
};
