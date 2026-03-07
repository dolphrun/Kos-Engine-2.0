#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PlayerManagerScript.h"

class ElementBaseScript : public TemplateSC {
public:
    // Reference to the player object
    utility::GUID playerObject;
    ecs::EntityID playerObjectID;

    // References to sprites for each power-up
    utility::GUID noneSprite;
    utility::GUID fireSprite;
    utility::GUID lightningSprite;
    utility::GUID acidSprite;

    void Start() override {
        // Resolve the entity ID from the player object GUID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Initialize the sprite on start
        UpdateAbilitySprite();
    }

    void Update() override {
        // Update sprite every frame based on current powerup
        UpdateAbilitySprite();
    }

private:
    void UpdateAbilitySprite() {
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity)) {

                // Check current powerup and set appropriate sprite
                switch (playerMgr->playerPowerupHeld) {
                case PlayerManagerScript::Powerup::FIRE:
                    sc->spriteGUID = fireSprite;
                    break;

                case PlayerManagerScript::Powerup::ACID:
                    sc->spriteGUID = acidSprite;
                    break;

                case PlayerManagerScript::Powerup::LIGHTNING:
                    sc->spriteGUID = lightningSprite;
                    break;

                case PlayerManagerScript::Powerup::NONE:
                default:
                    sc->spriteGUID = noneSprite;
                    break;
                }
            }
        }
    }

public:
    REFLECTABLE(ElementBaseScript, playerObject, playerObjectID,
        noneSprite, fireSprite, lightningSprite, acidSprite);
};