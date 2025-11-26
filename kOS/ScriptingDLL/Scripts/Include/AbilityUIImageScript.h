#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"

class AbilityUIImageScript : public TemplateSC {
public:
    std::string currAbility = "NONE"; // Tracks current power-up
    std::string lastAbility = "";     // Tracks last power-up to avoid redundant updates

    utility::GUID playerObject;       // Reference to the player object
    ecs::EntityID playerObjectID;

    // References to sprites for each power-up
    utility::GUID noneSprite;
    utility::GUID fireSprite;
    utility::GUID lightningSprite;
    utility::GUID acidSprite;
    utility::GUID fireLightningSprite;
    utility::GUID fireAcidSprite;
    utility::GUID lightningAcidSprite;

    void Start() override {
        // Resolve the entity ID from the player object GUID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Initialize the sprite on start
        UpdateAbilitySprite();
    }

    void Update() override {
        // Fetch the current power-up from PlayerManagerScript
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            if (playerMgr->playerPowerupHeld == 0) {
                currAbility = "NONE";
            }
            else if (playerMgr->playerPowerupHeld == 1) {
                currAbility = "FIRE";
            }
            else if (playerMgr->playerPowerupHeld == 2) {
                currAbility = "ACID";
            }
            else if (playerMgr->playerPowerupHeld == 3) {
                currAbility = "LIGHTNING";
            }
            else if (playerMgr->playerPowerupHeld == 4) {
                currAbility = "FIREACID";
            }
            else if (playerMgr->playerPowerupHeld == 5) {
                currAbility = "FIRELIGHTNING";
            }
            else if (playerMgr->playerPowerupHeld == 6) {
                currAbility = "ACIDLIGHTNING";
            }
        }

        // Only update the sprite if the ability changed
        if (currAbility != lastAbility) {
            UpdateAbilitySprite();
            lastAbility = currAbility;
        }
    }

private:
    void UpdateAbilitySprite() {
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity)) {
            if (currAbility == "FIRE") sc->spriteGUID = fireSprite;
            else if (currAbility == "LIGHTNING") sc->spriteGUID = lightningSprite;
            else if (currAbility == "ACID") sc->spriteGUID = acidSprite;
            else if (currAbility == "FIRELIGHTNING") sc->spriteGUID = fireLightningSprite;
            else if (currAbility == "FIREACID") sc->spriteGUID = fireAcidSprite;
            else if (currAbility == "ACIDLIGHTNING") sc->spriteGUID = lightningAcidSprite;
            else sc->spriteGUID = noneSprite; // Default for "none" or unknown abilities
        }
    }

public:
    REFLECTABLE(AbilityUIImageScript, currAbility, lastAbility, playerObject, playerObjectID,
        noneSprite, fireSprite, lightningSprite, acidSprite,
        fireLightningSprite, fireAcidSprite, lightningAcidSprite);
};
