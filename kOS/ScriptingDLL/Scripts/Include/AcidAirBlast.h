#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

class EnemyManagerScript;

class AcidAirBlast : public TemplateSC {
public:

    int     blastDamage = 30;
	float   pushbackForce = 30.f;
	float   staggerDuration = 1.f;
	int     manaCost = 30;
	int     scoreValue = 150;

	utility::GUID blastSfxGUID;

	ScoreManagerScript* scoreManager = nullptr;

    float   lifetime = 1.5f;
    float   currLifetime = 0.f;
    bool    isDying = false;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;

	std::vector<utility::GUID> enemyDeathSfxGUIDs;

    void Start() override {

        if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
            for (auto& af : ac->audioFiles) {
                if (af.audioGUID == blastSfxGUID && af.isSFX) {
                    af.requestPlay = true;
                    break;
                }
            }
        }

        for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
            if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
                scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
                break;
            }
        }

        auto damagedEnemies = std::make_shared<std::unordered_set<ecs::EntityID>>();

        physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this, damagedEnemies](const physics::Collision& col) {

            if (damagedEnemies->count(col.otherEntityID)) return;

            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp || nameComp->entityTag != "Enemy") return;

            auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
            if (!enemyScript) return;

            auto* selfTransform = ecsPtr->GetComponent<TransformComponent>(entity);
            auto* enemyTransform = ecsPtr->GetComponent<TransformComponent>(col.otherEntityID);
            if (!selfTransform || !enemyTransform) return;

            damagedEnemies->insert(col.otherEntityID);

            glm::vec3 pushDir = enemyTransform->WorldTransformation.position
                - selfTransform->WorldTransformation.position;

            enemyScript->TakeDamage(blastDamage, "ACID");

            // Only stagger and push back if the shield is broken (or if it never had one)
            if (enemyScript->shieldHealth <= 0) {
                enemyScript->TriggerStagger(staggerDuration);
                enemyScript->ApplyPushback(pushDir, pushbackForce);
            }

            std::cout << "[AirBlast] Hit enemy | HP left: " << enemyScript->enemyHealth << "\n";

            if (enemyScript->enemyHealth <= 0) {
                if (scoreManager) {
                    scoreManager->AddScore(scoreValue);
                }
                enemyScript->Die(); 
            }
            });
        currLifetime = lifetime;
        isDying = true;

    }

    void Update() override {

        if (isDying) {
            currLifetime -= ecsPtr->m_GetDeltaTime();

            if (currLifetime <= 0.f) {
                ecsPtr->DeleteEntity(entity);
            }
        }
    }
    REFLECTABLE(AirBlastScript, blastDamage, pushbackForce, staggerDuration, manaCost, scoreValue, blastSfxGUID)
};
