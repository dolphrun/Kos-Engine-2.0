#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"
#include "PlayerManagerScript.h"

class EnemyHealthScript : public TemplateSC {
public:
    // --- Player Reference ---
    utility::GUID playerObject;
    ecs::EntityID playerObjectID;

    // --- Raycast Settings ---
    float raycastRange = 30.0f;

    // --- HUD Child Entities (assign in editor) ---
    utility::GUID healthBarFillEntity;      // Enemy Healthbar (scales with HP)
    ecs::EntityID healthBarFillID;

    utility::GUID resistanceEntity;         // Enemy Resistance (single sprite that swaps)
    ecs::EntityID resistanceEntityID;

    utility::GUID afflictionEntity;         // Enemy Affliction Status (single sprite that swaps)
    ecs::EntityID afflictionEntityID;

    // --- Resistance Sprites (assign the PNGs in editor) ---
    utility::GUID fireResistSprite;         // Fire_Resistant.png
    utility::GUID acidResistSprite;         // Acid_Resistant.png
    utility::GUID lightningResistSprite;    // Electric_Resistant.png

    // --- Affliction Sprites (assign the PNGs in editor) ---
    utility::GUID fireAfflictedSprite;      // Fire_Afflicted.png
    utility::GUID acidAfflictedSprite;      // Acid_Afflicted.png
    utility::GUID lightningAfflictedSprite; // Electric_Afflicted.png

    // --- Internal State ---
    glm::vec3 healthBarOriginalScale = glm::vec3(1.f);
    glm::vec3 healthBarOriginalPosition = glm::vec3(0.f);
    bool healthBarInitialized = false;

    void Start() override {
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);
        healthBarFillID = ecsPtr->GetEntityIDFromGUID(healthBarFillEntity);
        resistanceEntityID = ecsPtr->GetEntityIDFromGUID(resistanceEntity);
        afflictionEntityID = ecsPtr->GetEntityIDFromGUID(afflictionEntity);

        // Hide resistance and affliction icons at start
        ecsPtr->SetActive(resistanceEntityID, false);
        ecsPtr->SetActive(afflictionEntityID, false);
    }

    void Update() override {
        auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID);
        if (!playerMgr) return;

        // Raycast from camera forward
        auto* camTransform = ecsPtr->GetComponent<TransformComponent>(playerMgr->playerCameraObjectID);
        if (!camTransform) return;

        glm::vec3 origin = camTransform->WorldTransformation.position;
        glm::vec3 direction = playerMgr->GetPlayerCameraFrontDirection();

        RaycastHit hit;
        bool didHit = physicsPtr->Raycast(origin, direction, raycastRange, hit, nullptr);

        if (didHit) {
            ecs::EntityID hitID = static_cast<ecs::EntityID>(hit.entityID);
            auto* enemyMgr = ecsPtr->GetComponent<EnemyManagerScript>(hitID);

            if (enemyMgr && !enemyMgr->isDead) {
                UpdateHealthBar(enemyMgr);
                UpdateResistanceSprite(enemyMgr);
                UpdateAfflictionSprite(enemyMgr);
                return;
            }
        }

        // Nothing valid hit — hide resistance and affliction icons
        // (health bar hiding is handled by the parent panel in your hierarchy)
        ecsPtr->SetActive(resistanceEntityID, false);
        ecsPtr->SetActive(afflictionEntityID, false);
    }

private:

    void UpdateHealthBar(EnemyManagerScript* enemyMgr) {
        auto* tc = ecsPtr->GetComponent<TransformComponent>(healthBarFillID);
        if (!tc) return;

        if (!healthBarInitialized) {
            healthBarOriginalScale = tc->LocalTransformation.scale;
            healthBarOriginalPosition = tc->LocalTransformation.position;
            healthBarInitialized = true;
        }

        // Replace 100.f with enemyMgr->maxEnemyHealth once your teammate adds it
        float healthPct = std::clamp((float)enemyMgr->enemyHealth / 100.f, 0.f, 1.f);

        tc->LocalTransformation.scale.x = healthBarOriginalScale.x * healthPct;
        tc->LocalTransformation.scale.y = healthBarOriginalScale.y;
        tc->LocalTransformation.scale.z = healthBarOriginalScale.z;

        float scaleReduction = healthBarOriginalScale.x * (1.f - healthPct);
        tc->LocalTransformation.position.x = healthBarOriginalPosition.x - (scaleReduction * 0.5f);
        tc->LocalTransformation.position.y = healthBarOriginalPosition.y;
        tc->LocalTransformation.position.z = healthBarOriginalPosition.z;
    }

    void UpdateResistanceSprite(EnemyManagerScript* enemyMgr) {
        auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(resistanceEntityID);
        if (!sc) return;

        // --- HOOK IN LATER ---
        // Replace these conditions with your teammate's resistance fields e.g:
        //   if (enemyMgr->isFireResistant)
        //   if (enemyMgr->isAcidResistant)
        //   if (enemyMgr->isLightningResistant)

        if (false) { // TODO: enemyMgr->isFireResistant
            sc->spriteGUID = fireResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else if (false) { // TODO: enemyMgr->isAcidResistant
            sc->spriteGUID = acidResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else if (false) { // TODO: enemyMgr->isLightningResistant
            sc->spriteGUID = lightningResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else {
            // No resistance — hide the icon
            ecsPtr->SetActive(resistanceEntityID, false);
        }
    }

    void UpdateAfflictionSprite(EnemyManagerScript* enemyMgr) {
        auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(afflictionEntityID);
        if (!sc) return;

        // --- HOOK IN LATER ---
        // Replace these conditions with your teammate's affliction fields e.g:
        //   if (enemyMgr->isOnFire)
        //   if (enemyMgr->isAcidAfflicted)
        //   if (enemyMgr->lightningStack > 0)   <- this one already exists!

        if (enemyMgr->lightningStack > 0) {  // already exists in EnemyManagerScript!
            sc->spriteGUID = lightningAfflictedSprite;
            ecsPtr->SetActive(afflictionEntityID, true);
        }
        else if (false) { // TODO: enemyMgr->isOnFire
            sc->spriteGUID = fireAfflictedSprite;
            ecsPtr->SetActive(afflictionEntityID, true);
        }
        else if (false) { // TODO: enemyMgr->isAcidAfflicted
            sc->spriteGUID = acidAfflictedSprite;
            ecsPtr->SetActive(afflictionEntityID, true);
        }
        else {
            // No affliction — hide the icon
            ecsPtr->SetActive(afflictionEntityID, false);
        }
    }

public:
    REFLECTABLE(EnemyHealthScript,
        playerObject, raycastRange,
        healthBarFillEntity,
        resistanceEntity, afflictionEntity,
        fireResistSprite, acidResistSprite, lightningResistSprite,
        fireAfflictedSprite, acidAfflictedSprite, lightningAfflictedSprite
    );
};