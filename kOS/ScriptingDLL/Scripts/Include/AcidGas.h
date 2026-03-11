#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

// FORWARD DECLARATION: Fixes circular dependency
class EnemyManagerScript;

class AcidGas : public TemplateSC {
public:
    int     gasDamage = 10;        // Damage per tick
    float   tickInterval = 1.f;  
    float   lifetime = 2.2f;
    int     scoreValue = 50;

    // INTERNAL
    float   currLifetime = 0.f;
    float   currTickTimer = 0.f;

    std::unordered_set<ecs::EntityID> enemiesInCloud;


    ScoreManagerScript* scoreManager = nullptr;


    // Declarations Only (Implementation at the bottom)
    void Start() override;
    void Update() override;

    REFLECTABLE(AcidGas, gasDamage, tickInterval, lifetime, scoreValue)
};

// --- IMPLEMENTATION SECTION ---
// Include EnemyManagerScript here so we can access enemyHealth
#include "EnemyManagerScript.h"

inline void AcidGas::Start() {
    currLifetime = lifetime;
    currTickTimer = tickInterval;

    for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
        if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
            scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
            break;
        }
    }

    physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
        if (currLifetime <= 0.f) return;

        auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
        if (!nameComp) return;
        if (nameComp->entityTag != "Enemy") return;

        enemiesInCloud.insert(col.otherEntityID);
        std::cout << "[AcidGasCloud] Enemy entered cloud. Count: " << enemiesInCloud.size() << "\n";
        });

    physicsPtr->GetEventCallback()->OnTriggerExit(entity, [this](const physics::Collision& col) {
        if (currLifetime <= 0.f) return;


        enemiesInCloud.erase(col.otherEntityID);
        std::cout << "[AcidGasCloud] Enemy exited cloud. Count: " << enemiesInCloud.size() << "\n";
        });
}

inline void AcidGas::Update() {
    if (currLifetime <= 0.f) return;

    float dt = ecsPtr->m_GetDeltaTime();

    currTickTimer -= dt;

    if (currTickTimer <= 0.f) {
        currTickTimer = tickInterval;

        std::vector<ecs::EntityID> toRemove;

        for (ecs::EntityID enemyID : enemiesInCloud) {

            if (!ecsPtr->IsValidEntity(enemyID)) {
                toRemove.push_back(enemyID);
                continue;
            }

            auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(enemyID);
            if (!enemyScript) {
                toRemove.push_back(enemyID);
                continue;
            }

            enemyScript->TakeDamage(gasDamage, "ACID");
            std::cout << "[AcidGasCloud] Tick damage to enemy | HP left: " << enemyScript->enemyHealth << "\n";

            if (enemyScript->enemyHealth <= 0) {
                if (scoreManager) {
                    scoreManager->AddScore(scoreValue);
                }
                enemyScript->Die();
                toRemove.push_back(enemyID);
            }
        }

        for (ecs::EntityID id : toRemove) {
            enemiesInCloud.erase(id);
        }
    }

    currLifetime -= dt;
    //if (currLifetime <= 0.f) {
    //    ecsPtr->DeleteEntity(entity);
    //}


    if (currLifetime <= 0.f) {
        // Clear deleting
        enemiesInCloud.clear();

        // Unregister physics callbacks
        physicsPtr->GetEventCallback()->DeregisterEntity(entity);
        physicsPtr->GetEventCallback()->DeregisterEntity(entity);

        ecsPtr->DeleteEntity(entity);

        return;
    }
}