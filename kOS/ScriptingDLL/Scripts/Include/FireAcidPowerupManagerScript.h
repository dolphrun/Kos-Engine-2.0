#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class FireAcidPowerupManagerScript : public TemplateSC {
public:
	int flamethrowerDamage = 1;
	float lingerTime;
	float timeBeforeDamageAgain;

	float currentTimer = 0.f;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	utility::GUID flameThrowerSfxGUID;

	void Start() override {
		enemyDeathSfxGUIDs.clear();
		if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
		if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
		if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

		// ADD SFX OF FLAMETHROWER HERE - Done
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == flameThrowerSfxGUID && af.isSFX) {
					af.requestPlay = true;
					break;
				}
			}
		}

		physicsPtr->GetEventCallback()->OnTriggerStay(entity, [this](const physics::Collision& col) {
			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
					if (enemyScript->timeBeforeDamageByFlamethrowerAgain <= 0.f) {
						enemyScript->enemyHealth -= flamethrowerDamage;

						enemyScript->timeBeforeDamageByFlamethrowerAgain = timeBeforeDamageAgain;
					}

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

	REFLECTABLE(FireAcidPowerupManagerScript, flamethrowerDamage, lingerTime, timeBeforeDamageAgain, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3, flameThrowerSfxGUID)
};