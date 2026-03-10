#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"
#include "PlayerManagerScript.h"
#include <iostream>
#include <unordered_map>

class EnemyHealthScript : public TemplateSC {
public:
    utility::GUID playerObject;
    ecs::EntityID playerObjectID = -1;
    float raycastRange = 30.0f;

    utility::GUID healthBarFillEntity;
    ecs::EntityID healthBarFillID = -1;

    utility::GUID healthBarFullEntity;
    ecs::EntityID healthBarFullID = -1;

    utility::GUID healthBarFrameEntity;
    ecs::EntityID healthBarFrameID = -1;

    utility::GUID resistanceEntity;
    ecs::EntityID resistanceEntityID = -1;

    utility::GUID affliction1Entity;
    ecs::EntityID affliction1EntityID = -1;
    utility::GUID affliction2Entity;
    ecs::EntityID affliction2EntityID = -1;
    utility::GUID affliction3Entity;
    ecs::EntityID affliction3EntityID = -1;

    utility::GUID fireResistSprite, acidResistSprite, lightningResistSprite;
    utility::GUID fireAfflictedSprite, acidAfflictedSprite, lightningAfflictedSprite;

    bool showDebugRay = true;

    glm::vec3 originalHUDPosition = glm::vec3(0.f);
    glm::vec3 hiddenPosition = glm::vec3(-10000.f, -10000.f, 0.f);
    bool hudPositionCached = false;
    bool isHUDVisible = false;

    glm::vec3 originalResistancePosition = glm::vec3(0.f);
    bool resistancePositionCached = false;
    bool isResistanceVisible = false;

    glm::vec3 healthBarOriginalScale = glm::vec3(1.f);
    glm::vec3 healthBarOriginalPosition = glm::vec3(0.f);
    bool healthBarInitialized = false;

    std::unordered_map<ecs::EntityID, float> enemyMaxHealthCache;

    void Start() override {
        playerObjectID = ecsPtr->GetEntityIDFromGUID(playerObject);
        healthBarFillID = ecsPtr->GetEntityIDFromGUID(healthBarFillEntity);
        healthBarFullID = ecsPtr->GetEntityIDFromGUID(healthBarFullEntity);
        healthBarFrameID = ecsPtr->GetEntityIDFromGUID(healthBarFrameEntity);
        resistanceEntityID = ecsPtr->GetEntityIDFromGUID(resistanceEntity);
        affliction1EntityID = ecsPtr->GetEntityIDFromGUID(affliction1Entity);
        affliction2EntityID = ecsPtr->GetEntityIDFromGUID(affliction2Entity);
        affliction3EntityID = ecsPtr->GetEntityIDFromGUID(affliction3Entity);

        SetHUDVisible(false);

        //if (playerObjectID == -1) std::cout << "[EnemyHealthScript] WARN: playerObject not resolved\n";
        //if (healthBarFillID == -1) std::cout << "[EnemyHealthScript] WARN: healthBarFillEntity not resolved\n";
        //if (healthBarFullID == -1) std::cout << "[EnemyHealthScript] WARN: healthBarFullEntity not resolved\n";
        //if (healthBarFrameID == -1) std::cout << "[EnemyHealthScript] WARN: healthBarFrameEntity not resolved\n";
        //if (resistanceEntityID == -1) std::cout << "[EnemyHealthScript] WARN: resistanceEntity not resolved\n";

        // Cache HUD original position
        auto* hudT = ecsPtr->GetComponent<TransformComponent>(entity);
        if (hudT) {
            originalHUDPosition = hudT->LocalTransformation.position;
            hudPositionCached = true;
        }

        // Cache resistance original position
        if (resistanceEntityID != -1) {
            auto* resistT = ecsPtr->GetComponent<TransformComponent>(resistanceEntityID);
            if (resistT) {
                originalResistancePosition = resistT->LocalTransformation.position;
                resistancePositionCached = true;
            }
        }

        // Cache health bar original transform
        if (healthBarFillID != -1) {
            auto* sc = ecsPtr->GetComponent<TransformComponent>(healthBarFillID);
            if (sc) {
                healthBarOriginalScale = sc->LocalTransformation.scale;
                healthBarOriginalPosition = sc->LocalTransformation.position;
                healthBarInitialized = true;
            }
        }

        // Hide HUD at start
        SetResistanceVisible(false);
    }

    void Update() override {
        if (playerObjectID == -1 || healthBarFillID == -1) return;

        auto* playerMgr = ecsPtr->GetComponent<PlayerManagerScript>(playerObjectID);
        if (!playerMgr) return;

        auto* camTransform = ecsPtr->GetComponent<TransformComponent>(playerMgr->playerCameraObjectID);
        if (!camTransform) return;

        glm::vec3 origin = camTransform->WorldTransformation.position;
        glm::vec3 direction = playerMgr->GetPlayerCameraFrontDirection();

        if (showDebugRay)
            physicsPtr->m_debugRays.push_back({ origin, origin + direction * raycastRange });

        auto* playerRb = ecsPtr->GetComponent<RigidbodyComponent>(playerObjectID);

        RaycastHit hit;
        bool didHit = physicsPtr->Raycast(origin, direction, raycastRange, hit, playerRb ? playerRb->actor : nullptr);

        if (didHit) {
            ecs::EntityID hitID = static_cast<ecs::EntityID>(hit.entityID);
            EnemyManagerScript* enemyMgr = nullptr;
            ecs::EntityID enemyEntityID = -1;

            auto isEnemyTag = [&](ecs::EntityID id) -> bool {
                auto* nameComp = ecsPtr->GetComponent<NameComponent>(id);
                return nameComp && nameComp->entityTag == "Enemy";
                };

            if (isEnemyTag(hitID)) {
                enemyMgr = ecsPtr->GetComponent<EnemyManagerScript>(hitID);
                enemyEntityID = hitID;
            }
            else {
                auto parent = ecsPtr->GetParent(hitID);
                if (parent.has_value() && isEnemyTag(parent.value())) {
                    enemyMgr = ecsPtr->GetComponent<EnemyManagerScript>(parent.value());
                    enemyEntityID = parent.value();
                }
            }

            if (enemyMgr && !enemyMgr->isDead) {
                std::cout << "[EnemyHealthScript] Hit enemy entityID=" << enemyEntityID
                    << " enemyHealth=" << enemyMgr->enemyHealth
                    << " isDead=" << enemyMgr->isDead << "\n";
                SetHUDVisible(true);
                UpdateHealthBar(enemyMgr, enemyEntityID);
                UpdateResistanceSprite(enemyMgr);
                return;
            }
        }

        // Not pointing at enemy - hide HUD
        SetHUDVisible(false);
    }

private:
    void SetHUDVisible(bool visible) {
        if (!hudPositionCached) return;
        if (isHUDVisible == visible) return;
        auto* t = ecsPtr->GetComponent<TransformComponent>(entity);
        if (!t) return;
        t->LocalTransformation.position = visible ? originalHUDPosition : hiddenPosition;
        isHUDVisible = visible;
    }

    void SetResistanceVisible(bool visible) {
        if (!resistancePositionCached) return;
        if (isResistanceVisible == visible) return;
        auto* t = ecsPtr->GetComponent<TransformComponent>(resistanceEntityID);
        if (!t) return;
        t->LocalTransformation.position = visible ? originalResistancePosition : hiddenPosition;
        isResistanceVisible = visible;
    }

    void UpdateHealthBar(EnemyManagerScript* enemyMgr, ecs::EntityID enemyEntityID) {
        if (!healthBarInitialized || healthBarFillID == -1) return;

        auto* sc = ecsPtr->GetComponent<TransformComponent>(healthBarFillID);
        if (!sc) return;

        // Only cache max health the first time we see this enemy
        if (enemyMaxHealthCache.find(enemyEntityID) == enemyMaxHealthCache.end()) {
            enemyMaxHealthCache[enemyEntityID] = (float)enemyMgr->enemyHealth;
        }

        float maxHealth = enemyMaxHealthCache[enemyEntityID];
        float healthPct = std::clamp((float)enemyMgr->enemyHealth / maxHealth, 0.f, 1.f);

        std::cout << "[EnemyHealthScript] HP=" << enemyMgr->enemyHealth
            << " max=" << maxHealth
            << " pct=" << healthPct << "\n";

        float newScaleX = healthBarOriginalScale.x * healthPct;
        float scaleReduction = healthBarOriginalScale.x - newScaleX;

        sc->LocalTransformation.scale.x = newScaleX;
        sc->LocalTransformation.position.x = healthBarOriginalPosition.x - (scaleReduction * 0.5f);
    }

    void UpdateResistanceSprite(EnemyManagerScript* enemyMgr) {
        if (!enemyMgr) return;
        auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(resistanceEntityID);
        if (!sc) return;
        // TODO: replace false with enemyMgr->isFireResistant etc. once your teammate adds them
        if (false) { // enemyMgr->isFireResistant
            sc->spriteGUID = fireResistSprite;
            SetResistanceVisible(true);
        }
        else if (false) { // enemyMgr->isAcidResistant
            sc->spriteGUID = acidResistSprite;
            SetResistanceVisible(true);
        }
        else if (false) { // enemyMgr->isLightningResistant
            sc->spriteGUID = lightningResistSprite;
            SetResistanceVisible(true);
        }
        else {
            SetResistanceVisible(false);
        }
    }

    void UpdateAfflictions(EnemyManagerScript* enemyMgr) {
        // TODO: implement once teammate adds affliction fields
        (void)enemyMgr;
    }

public:
    REFLECTABLE(EnemyHealthScript,
        playerObject, raycastRange, showDebugRay,
        healthBarFillEntity, healthBarFullEntity, healthBarFrameEntity,
        resistanceEntity,
        affliction1Entity, affliction2Entity, affliction3Entity,
        fireResistSprite, acidResistSprite, lightningResistSprite,
        fireAfflictedSprite, acidAfflictedSprite, lightningAfflictedSprite
    );
};