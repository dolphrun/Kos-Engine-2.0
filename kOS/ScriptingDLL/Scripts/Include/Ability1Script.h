#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PlayerManagerScript.h"

class Ability1Script : public TemplateSC {
public:
    // ===== CONFIGURATION =====
    // Which ability slot is this? (1=LMB, 2=RMB, 3=F key)
    int abilitySlot = 1;  // Default to slot 1

    // Reference to the player object
    utility::GUID playerObject;
    ecs::EntityID playerObjectID;

    // ===== SPRITES FOR EACH POWERUP =====

    // Default/Base sprites (used when no powerup OR for base weapon in slot 1)
    utility::GUID defaultSprite;           // For slots 2 & 3 when no powerup
    utility::GUID baseActiveSprite;        // For slot 1 (LMB) base weapon ready
    utility::GUID baseInactiveSprite;      // For slot 1 (LMB) base weapon not ready

    // Fire powerup sprites
    utility::GUID fireActiveSprite;
    utility::GUID fireInactiveSprite;

    // Acid powerup sprites
    utility::GUID acidActiveSprite;
    utility::GUID acidInactiveSprite;

    // Lightning powerup sprites
    utility::GUID lightningActiveSprite;
    utility::GUID lightningInactiveSprite;

    void Start() override {
        // Resolve the player entity ID
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);

        // Validate
        if (playerObjectID == 0) {
            std::cout << "[AbilityUIScript] ERROR: Could not resolve player entity!" << std::endl;
            std::cout << "  Ability Slot: " << abilitySlot << std::endl;
            return;
        }

        std::cout << "[AbilityUIScript] Start - Slot " << abilitySlot << std::endl;

        // Initialize sprite
        UpdateAbilitySprite();
    }

    void Update() override {
        // Safety check
        if (playerObjectID == 0) {
            return;
        }

        UpdateAbilitySprite();
    }

private:
    void UpdateAbilitySprite() {
        // Safety checks
        if (!ecsPtr->HasComponent<ecs::SpriteComponent>(entity)) {
            return;
        }

        if (!ecsPtr->HasComponent<PlayerManagerScript>(playerObjectID)) {
            return;
        }

        auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID);
        if (!playerMgr) {
            return;
        }

        auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(entity);
        if (!sc) {
            return;
        }

        // Route to correct ability handler based on slot
        switch (abilitySlot) {
        case 1:
            UpdateAbility1_LMB(playerMgr, sc);
            break;
        case 2:
            UpdateAbility2_RMB(playerMgr, sc);
            break;
        case 3:
            UpdateAbility3_F(playerMgr, sc);
            break;
        default:
            std::cout << "[AbilityUIScript] ERROR: Invalid abilitySlot: " << abilitySlot << std::endl;
            break;
        }
    }

    // ===== ABILITY 1 - LMB (Fire/Shooting) =====
    void UpdateAbility1_LMB(PlayerManagerScript* playerMgr, ecs::SpriteComponent* sc) {
        switch (playerMgr->playerPowerupHeld) {

        case PlayerManagerScript::Powerup::FIRE: {
            // Fire melee (LMB) - check cooldown only
            bool canUse = (playerMgr->fireCurrMeleeCooldown <= 0.0f);
            sc->spriteGUID = canUse ? fireActiveSprite : fireInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::ACID: {
            // Acid shooting (LMB) - check bullets, cooldown, and not reloading
            bool canUse = (playerMgr->acidCurrBullets > 0) &&
                (playerMgr->acidCurrShootCooldown <= 0.0f) &&
                !playerMgr->isReloading;
            sc->spriteGUID = canUse ? acidActiveSprite : acidInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::LIGHTNING: {
            // Lightning shooting (LMB) - check bullets, cooldown, and not reloading
            bool canUse = (playerMgr->lightningCurrBullets > 0) &&
                (playerMgr->lightningCurrShootCooldown <= 0.0f) &&
                !playerMgr->isReloading;
            sc->spriteGUID = canUse ? lightningActiveSprite : lightningInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::NONE:
        default: {
            // Base weapon - check bullets, cooldown, and not reloading
            bool canUse = (playerMgr->baseCurrBullets > 0) &&
                (playerMgr->baseCurrShootCooldown <= 0.0f) &&
                !playerMgr->isReloading;
            sc->spriteGUID = canUse ? baseActiveSprite : baseInactiveSprite;
            break;
        }
        }
    }

    // ===== ABILITY 2 - RMB (Special Ability) =====
    void UpdateAbility2_RMB(PlayerManagerScript* playerMgr, ecs::SpriteComponent* sc) {
        switch (playerMgr->playerPowerupHeld) {

        case PlayerManagerScript::Powerup::FIRE: {
            // Fire ability (RMB) - check if player has enough mana
            bool canUse = (playerMgr->currMana >= playerMgr->fireAbilityCost);
            sc->spriteGUID = canUse ? fireActiveSprite : fireInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::ACID: {
            // Acid ability (RMB) - check if player has enough mana
            bool canUse = (playerMgr->currMana >= playerMgr->acidAbilityCost);
            sc->spriteGUID = canUse ? acidActiveSprite : acidInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::LIGHTNING: {
            // Lightning ability (RMB) - check if player has enough mana
            bool canUse = (playerMgr->currMana >= playerMgr->lightningAbilityCost);
            sc->spriteGUID = canUse ? lightningActiveSprite : lightningInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::NONE:
        default:
            // No powerup - show default image
            sc->spriteGUID = defaultSprite;
            break;
        }
    }

    // ===== ABILITY 3 - F (Dash/Movement) =====
    void UpdateAbility3_F(PlayerManagerScript* playerMgr, ecs::SpriteComponent* sc) {
        switch (playerMgr->playerPowerupHeld) {

        case PlayerManagerScript::Powerup::FIRE: {
            // Fire dash (F key) - check mana AND cooldown
            bool canUse = (playerMgr->currMana >= playerMgr->fireMovementCost) &&
                (playerMgr->fireCurrMovementCooldown <= 0.0f);
            sc->spriteGUID = canUse ? fireActiveSprite : fireInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::ACID: {
            // Acid movement (F key) - check mana AND cooldown
            bool canUse = (playerMgr->currMana >= playerMgr->acidMovementCost) &&
                (playerMgr->acidCurrMovementCooldown <= 0.0f);
            sc->spriteGUID = canUse ? acidActiveSprite : acidInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::LIGHTNING: {
            // Lightning blink (F key) - check mana AND cooldown
            bool canUse = (playerMgr->currMana >= playerMgr->lightningMovementCost) &&
                (playerMgr->lightningCurrMovementCooldown <= 0.0f);
            sc->spriteGUID = canUse ? lightningActiveSprite : lightningInactiveSprite;
            break;
        }

        case PlayerManagerScript::Powerup::NONE:
        default:
            // No powerup - show default image (no dash available)
            sc->spriteGUID = defaultSprite;
            break;
        }
    }

public:
    REFLECTABLE(Ability1Script,
        abilitySlot, playerObject, playerObjectID,
        defaultSprite, baseActiveSprite, baseInactiveSprite,
        fireActiveSprite, fireInactiveSprite,
        acidActiveSprite, acidInactiveSprite,
        lightningActiveSprite, lightningInactiveSprite);
};