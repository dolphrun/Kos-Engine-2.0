#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FireLightningPowerupManagerScript : public TemplateSC {
public:
	int groundSpikesDamage = 5;
	float lingerTime;

	float currentTimer;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	utility::GUID groundSpikeSfxGUID;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	void Start() override {
		enemyDeathSfxGUIDs.clear();
		if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
		if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
		if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

		// ADD SFX OF GROUNDSPIKES HERE - Done
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == groundSpikeSfxGUID && af.isSFX) {
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

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
					enemyScript->enemyHealth -= groundSpikesDamage;

					if (enemyScript->enemyHealth <= 0) {
						// ADD SFX OF ENEMY DEATH HERE - Done
						PlayRandomEnemyDeathSFX();

						ecsPtr->DeleteEntity(col.otherEntityID);
						navMeshPtr->RemoveAgent(enemyScript->agentid);
					}
				}
			}
			});
	}

	void Update() override {
		if (currentTimer <= lingerTime) {
			currentTimer += ecsPtr->m_GetDeltaTime();

			if (currentTimer <= lingerTime / 5.f) {
				for (int i = 0; i < ecsPtr->GetComponent<TransformComponent>(entity)->m_childID.size(); ++i) {
					if (auto* childTransform = ecsPtr->GetComponent<TransformComponent>(ecsPtr->GetComponent<TransformComponent>(entity)->m_childID[i])) {
						childTransform->LocalTransformation.position.y += ecsPtr->m_GetDeltaTime() * 45.f;
					}
				}
			}

			if (currentTimer >= lingerTime) {
				ecsPtr->DeleteEntity(entity);
			}
		}
	}

	void PlayRandomEnemyDeathSFX()
	{
		auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity);
		if (!ac) return;

		std::vector<ecs::AudioFile*> enemyDeathSfxPool;

		for (auto& af : ac->audioFiles) {
			if (!af.isSFX) continue;

			for (auto& g : enemyDeathSfxGUIDs) {
				if (!g.Empty() && af.audioGUID == g) {
					enemyDeathSfxPool.push_back(&af);
					break;
				}
			}
		}

		if (enemyDeathSfxPool.empty()) {
			std::cout << "[BulletLogic] No enemy death SFX found.\n";
			return;
		}

		int idx = rand() % static_cast<int>(enemyDeathSfxPool.size());
		enemyDeathSfxPool[idx]->requestPlay = true;

		std::cout << "[BulletLogic] Playing enemy death SFX index " << idx << "\n";
	}

	REFLECTABLE(FireLightningPowerupManagerScript, groundSpikesDamage, lingerTime, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3, groundSpikeSfxGUID)
};