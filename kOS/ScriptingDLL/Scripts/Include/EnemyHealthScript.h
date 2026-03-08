#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"
#include "PlayerManagerScript.h"

class EnemyHealthScript : public TemplateSC {
public:
    // --- Player Reference ---
    utility::GUID playerObject;
    ecs::EntityID playerObjectID = -1;

    // --- Raycast Settings ---
    float raycastRange = 30.0f;

    // --- HUD Child Entities (assign in editor) ---
    utility::GUID healthBarFillEntity;      // Enemy Healthbar (scales with HP)
    ecs::EntityID healthBarFillID = -1;

    utility::GUID resistanceEntity;         // Enemy Resistance (swaps sprite)
    ecs::EntityID resistanceEntityID = -1;

    // Affliction entities - stubbed for now, assign in editor when ready
    utility::GUID affliction1Entity;
    ecs::EntityID affliction1EntityID = -1;

    utility::GUID affliction2Entity;
    ecs::EntityID affliction2EntityID = -1;

    utility::GUID affliction3Entity;
    ecs::EntityID affliction3EntityID = -1;

    // --- Resistance Sprites (assign PNGs in editor) ---
    utility::GUID fireResistSprite;
    utility::GUID acidResistSprite;
    utility::GUID lightningResistSprite;

    // --- Affliction Sprites (assign PNGs in editor when ready) ---
    utility::GUID fireAfflictedSprite;
    utility::GUID acidAfflictedSprite;
    utility::GUID lightningAfflictedSprite;

    // --- Debug ---
    bool showDebugRay = true;

    // --- Internal State ---
    glm::vec3 healthBarOriginalScale = glm::vec3(1.f);
    glm::vec3 healthBarOriginalPosition = glm::vec3(0.f);
    bool healthBarInitialized = false;

    void Start() override {
        LOGGING_INFO("EnemyHealthScript: Start() called");

        if (!ecsPtr) { LOGGING_ERROR("EnemyHealthScript: ecsPtr is null!");     return; }
        if (!physicsPtr) { LOGGING_ERROR("EnemyHealthScript: physicsPtr is null!"); return; }

        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);
        healthBarFillID = ecsPtr->GetEntityIDFromGUID(healthBarFillEntity);
        resistanceEntityID = ecsPtr->GetEntityIDFromGUID(resistanceEntity);
        affliction1EntityID = ecsPtr->GetEntityIDFromGUID(affliction1Entity);
        affliction2EntityID = ecsPtr->GetEntityIDFromGUID(affliction2Entity);
        affliction3EntityID = ecsPtr->GetEntityIDFromGUID(affliction3Entity);

        if (playerObjectID == -1) LOGGING_WARN("EnemyHealthScript: playerObject GUID not assigned!");
        if (healthBarFillID == -1) LOGGING_WARN("EnemyHealthScript: healthBarFillEntity GUID not assigned!");
        if (resistanceEntityID == -1) LOGGING_WARN("EnemyHealthScript: resistanceEntity GUID not assigned!");

        // Hide resistance at start — afflictions skipped until implemented
        if (resistanceEntityID != -1) ecsPtr->SetActive(resistanceEntityID, false);

        LOGGING_INFO("EnemyHealthScript: Start() complete");
    }

    void Update() override {
        if (playerObjectID == -1 || healthBarFillID == -1 || resistanceEntityID == -1) {
            LOGGING_WARN("EnemyHealthScript: IDs not resolved, skipping update");
            return;
        }

        auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID);
        if (!playerMgr) { LOGGING_WARN("EnemyHealthScript: playerMgr is null"); return; }

        auto* camTransform = ecsPtr->GetComponent<TransformComponent>(playerMgr->playerCameraObjectID);
        if (!camTransform) { LOGGING_WARN("EnemyHealthScript: camTransform is null"); return; }

        glm::vec3 origin = camTransform->WorldTransformation.position;
        glm::vec3 direction = playerMgr->GetPlayerCameraFrontDirection();

        if (showDebugRay) {
            physicsPtr->m_debugRays.push_back({ origin, origin + direction * raycastRange });
        }

        RaycastHit hit;
        bool didHit = physicsPtr->Raycast(origin, direction, raycastRange, hit, nullptr);

        LOGGING_INFO("EnemyHealthScript: didHit={} hitEntityID={}", didHit, didHit ? hit.entityID : -1);

        if (didHit) {
            ecs::EntityID hitID = static_cast<ecs::EntityID>(hit.entityID);

            auto* enemyMgr = ecsPtr->GetComponent<EnemyManagerScript>(hitID);

            // Try parent if not found directly
            if (!enemyMgr) {
                auto parent = ecsPtr->GetParent(hitID);
                if (parent.has_value()) {
                    LOGGING_INFO("EnemyHealthScript: EnemyManagerScript not on hitID={}, trying parent={}", hitID, parent.value());
                    enemyMgr = ecsPtr->GetComponent<EnemyManagerScript>(parent.value());
                }
            }

            if (enemyMgr) {
                LOGGING_INFO("EnemyHealthScript: Found EnemyManagerScript! enemyHealth={} isDead={}", enemyMgr->enemyHealth, enemyMgr->isDead);
            }
            else {
                LOGGING_WARN("EnemyHealthScript: No EnemyManagerScript found on hitID={} or its parent", hitID);
            }

            if (enemyMgr && !enemyMgr->isDead) {
                UpdateHealthBar(enemyMgr);
                UpdateResistanceSprite(enemyMgr);
                return;
            }
        }

        if (resistanceEntityID != -1) ecsPtr->SetActive(resistanceEntityID, false);
    }

private:

    void UpdateHealthBar(EnemyManagerScript* enemyMgr) {
        if (!enemyMgr) return;
        auto* tc = ecsPtr->GetComponent<TransformComponent>(healthBarFillID);
        if (!tc) return;

        if (!healthBarInitialized) {
            healthBarOriginalScale = tc->LocalTransformation.scale;
            healthBarOriginalPosition = tc->LocalTransformation.position;
            healthBarInitialized = true;
        }

        // TODO: replace 100.f with enemyMgr->maxEnemyHealth once your teammate adds it
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
        if (!enemyMgr) return;
        auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(resistanceEntityID);
        if (!sc) return;

        // TODO: replace false with enemyMgr->isFireResistant etc. once your teammate adds them
        if (false) { // enemyMgr->isFireResistant
            sc->spriteGUID = fireResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else if (false) { // enemyMgr->isAcidResistant
            sc->spriteGUID = acidResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else if (false) { // enemyMgr->isLightningResistant
            sc->spriteGUID = lightningResistSprite;
            ecsPtr->SetActive(resistanceEntityID, true);
        }
        else {
            ecsPtr->SetActive(resistanceEntityID, false);
        }
    }

    // --- STUBBED: hook in when afflictions are implemented ---
    void UpdateAfflictions(EnemyManagerScript* enemyMgr) {
        // TODO: when your teammate adds affliction fields, implement this
        // and wire up affliction1EntityID, affliction2EntityID, affliction3EntityID
        // e.g:
        //   if (enemyMgr->isOnFire)         -> show affliction1, set sprite to fireAfflictedSprite
        //   if (enemyMgr->isAcidAfflicted)  -> show affliction2, set sprite to acidAfflictedSprite
        //   if (enemyMgr->lightningStack>0) -> show affliction3, set sprite to lightningAfflictedSprite
    }

public:
    REFLECTABLE(EnemyHealthScript,
        playerObject, raycastRange, showDebugRay,
        healthBarFillEntity,
        resistanceEntity,
        affliction1Entity, affliction2Entity, affliction3Entity,
        fireResistSprite, acidResistSprite, lightningResistSprite,
        fireAfflictedSprite, acidAfflictedSprite, lightningAfflictedSprite
    );
};