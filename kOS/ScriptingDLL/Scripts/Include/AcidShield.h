#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "EnemyManagerScript.h"

class AcidShield : public TemplateSC {
public:
	//int acidDamage = 5;
	float shieldDuration = 3.0f;
	int shieldDamage = 5;
	float dotTickRate = 1.0f;

	float currentTimer = 0.0f;
	float currentTickTimer = 0.f;

	bool shieldActive = false;

	float slowMultiplier = 0.6f;      // 0.5 = 50% speed

	std::vector<ecs::EntityID> enemiesInside;

	glm::vec3 direction;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;

	std::unordered_map<ecs::EntityID, float> enemyTickTimers;

	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 200;

	void Start() override {
		shieldActive = true;    
		currentTimer = 0.f;    
		currentTickTimer = 0.f;

		enemyDeathSfxGUIDs.clear();
		if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
		if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
		if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

		//// ADD SFX OF ACID SPRAY HERE - Done
		//if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

		//	for (auto& af : ac->audioFiles) {
		//		if (af.audioGUID == acidSpraySfxGUID && af.isSFX) {
		//			af.requestPlay = true;
		//			break;
		//		}
		//	}
		//}

		for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
			if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
				scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
				break;
			}
		}

		physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
			if (!shieldActive) return;
			auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
			if (!nameComp || nameComp->entityTag != "Enemy") return;

			enemiesInside.push_back(col.otherEntityID);

			if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
				enemyScript->enemyMovementSpeed *= slowMultiplier;
			}

			/*		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
						if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {

						}

						auto* enemyRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(col.otherEntityID);
						if (!enemyRigidbody) return;

						glm::vec3 force = direction * pushbackForce;
						physicsPtr->AddForce(enemyRigidbody->actor, force, ForceMode::Impulse);
						std::cout << "ENEMY PUSH BACK: " << force.x << ", " << force.y << ", " << force.z << std::endl;
					}*/

			});

		
		//physicsPtr->GetEventCallback()->OnTriggerStay(entity, [this](const physics::Collision& col) {

		//	if (!shieldActive) return;

		//	auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		//	if (!nameComp || nameComp->entityTag != "Enemy") return;

		//	if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {

		//		currentTickTimer += ecsPtr->m_GetDeltaTime();

		//		if (currentTickTimer >= dotTickRate) {
		//			currentTickTimer = 0.f;

		//			ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth -= shieldDamage;
		//			std::cout << "[AcidShield] DOT tick! Enemy health now: "
		//				<< ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyHealth << "\n";
		//		}
		//	}
		//});

		physicsPtr->GetEventCallback()->OnTriggerStay(entity, [this](const physics::Collision& col) {
			if (!shieldActive) return;

			auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
			if (!nameComp || nameComp->entityTag != "Enemy") return;

			// Each enemy gets its own tick timer
			enemyTickTimers[col.otherEntityID] += ecsPtr->m_GetDeltaTime();

			if (enemyTickTimers[col.otherEntityID] >= dotTickRate) {
				enemyTickTimers[col.otherEntityID] = 0.f;

				if (auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)) {
					enemyScript->TakeDamage(shieldDamage, "ACID");

					if (enemyScript->shieldHealth <= 0) {
						enemyScript->TriggerStagger(0.5f);
					}
				}
			}
			});



		// Enemy leaves bubble, restore speed
		physicsPtr->GetEventCallback()->OnTriggerExit(entity, [this](const physics::Collision& col) {

			if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {

				enemiesInside.erase(
					std::remove(enemiesInside.begin(), enemiesInside.end(), col.otherEntityID),
					enemiesInside.end()
				);


				ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID)->enemyMovementSpeed /= slowMultiplier;

				//std::cout << "[AcidShield] Enemy left bubble. Speed restored!\n";
			}
			});

	}

	void Update() override {

		if (!shieldActive) return;

		currentTimer += ecsPtr->m_GetDeltaTime();
		if (currentTimer >= shieldDuration) {
			ExpireShield();
			return;
		}
	}

	inline void ExpireShield() {
		shieldActive = false;

		for (ecs::EntityID id : enemiesInside) {

			ecsPtr->GetComponent<EnemyManagerScript>(id)->enemyMovementSpeed /= slowMultiplier;
		}

		enemiesInside.clear();

		//std::cout << "[AcidShield] Shield expired. Cleaning up.\n";
		ecsPtr->DeleteEntity(entity);
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




	REFLECTABLE(AcidShield, shieldDuration, shieldDamage, dotTickRate, slowMultiplier, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3)
};