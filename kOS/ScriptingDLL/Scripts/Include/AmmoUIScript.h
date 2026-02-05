#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PlayerManagerScript.h"

class AmmoUIScript : public TemplateSC {
public:
    // Reference to the player object to read ammo from
    utility::GUID playerObject;
    ecs::EntityID playerObjectID;

    // Sprites for each ammo count (0 bullets to 6 bullets)
    utility::GUID ammo0Sprite;  // Empty
    utility::GUID ammo1Sprite;  // 1 bullet
    utility::GUID ammo2Sprite;  // 2 bullets
    utility::GUID ammo3Sprite;  // 3 bullets
    utility::GUID ammo4Sprite;  // 4 bullets
    utility::GUID ammo5Sprite;  // 5 bullets
    utility::GUID ammo6Sprite;  // 6 bullets (full)

    void Start() override {
        // Resolve the player entity ID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Initialize with current ammo
        UpdateAmmoSprite();
    }

    void Update() override {
        // Update sprite every frame based on current ammo
        UpdateAmmoSprite();
    }

private:
    void UpdateAmmoSprite() {
        if (auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID)) {
            if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity)) {

                // Get current bullets based on what weapon/powerup player has
                int currentBullets = playerMgr->GetCurrBulletsForCurrentWeapon();

                // Clamp to 0-6 range (just in case)
                currentBullets = std::clamp(currentBullets, 0, 6);

                // Set the appropriate sprite based on ammo count
                switch (currentBullets) {
                case 0:
                    sc->spriteGUID = ammo0Sprite;
                    break;
                case 1:
                    sc->spriteGUID = ammo1Sprite;
                    break;
                case 2:
                    sc->spriteGUID = ammo2Sprite;
                    break;
                case 3:
                    sc->spriteGUID = ammo3Sprite;
                    break;
                case 4:
                    sc->spriteGUID = ammo4Sprite;
                    break;
                case 5:
                    sc->spriteGUID = ammo5Sprite;
                    break;
                case 6:
                default:
                    sc->spriteGUID = ammo6Sprite;
                    break;
                }
            }
        }
    }

public:
    REFLECTABLE(AmmoUIScript, playerObject, playerObjectID,
        ammo0Sprite, ammo1Sprite, ammo2Sprite, ammo3Sprite,
        ammo4Sprite, ammo5Sprite, ammo6Sprite);
};