#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"

// FORWARD DECLARATION: Fixes circular dependency
class EnemyManagerScript;

class AcidLMB : public TemplateSC {
public:
	int acidLMBDamage = 10;

	glm::vec3 velocity = glm::vec3(0.f);
	float     launchSpeed = 15.f;
	float     arcUpwardKick = 5.f;
	float     gravity = -18.f;

	float     timeBeforeDeath = 2.5f;
	float     currentTimer = 0.f;

	bool      hasLanded = false; 

	glm::vec3 direction;


	ScoreManagerScript* scoreManager = nullptr;
	int scoreValue = 100;

	utility::GUID enemyDeathSfxGUID_1;
	utility::GUID enemyDeathSfxGUID_2;
	utility::GUID enemyDeathSfxGUID_3;
	std::vector<utility::GUID> enemyDeathSfxGUIDs;
	utility::GUID gasCloudPrefab;

	utility::GUID grenadeExplosionSfxGUID;

	// Declarations Only (Implementation at the bottom)
	void Start() override;
	void Update() override;
	void PlayRandomEnemyDeathSFX();
	void SpawnGasCloud();

	REFLECTABLE(AcidLMB, acidLMBDamage, launchSpeed, arcUpwardKick, gravity,
		timeBeforeDeath, enemyDeathSfxGUID_1, enemyDeathSfxGUID_2, enemyDeathSfxGUID_3, grenadeExplosionSfxGUID,gasCloudPrefab)
};

// --- IMPLEMENTATION SECTION ---
// Include EnemyManagerScript here so we can access enemyHealth
#include "EnemyManagerScript.h"
#include "AcidGas.h"

inline void AcidLMB::Start() {
	enemyDeathSfxGUIDs.clear();
	if (!enemyDeathSfxGUID_1.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_1);
	if (!enemyDeathSfxGUID_2.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_2);
	if (!enemyDeathSfxGUID_3.Empty()) enemyDeathSfxGUIDs.push_back(enemyDeathSfxGUID_3);

		for (const auto& [entityID, signature] : ecsPtr->GetEntitySignatureData()) {
			if (ecsPtr->HasComponent<ScoreManagerScript>(entityID)) {
				scoreManager = ecsPtr->GetComponent<ScoreManagerScript>(entityID);
				break;
			}
		}

	physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
		if (hasLanded) return;

		auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
		if (!nameComp) return;

		if (ecsPtr->GetComponent<NameComponent>(col.otherEntityID)->entityTag == "Enemy") {
			SpawnGasCloud();

			auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(col.otherEntityID);
			if (!enemyScript) return;

			// ADD SFX OF ENEMY DEATH HERE - DONE
			PlayRandomEnemyDeathSFX();

			enemyScript->TakeDamage(acidLMBDamage, "ACID");
			

			if (enemyScript->enemyHealth <= 0) {
				if (scoreManager) {
					scoreManager->AddScore(scoreValue);
				}
				enemyScript->Die();
			}

			hasLanded = true;
			ecsPtr->DeleteEntity(entity);
		}

		if (nameComp->entityTag == "Ground" || nameComp->entityTag == "Default") {
			hasLanded = true;
			SpawnGasCloud();
			ecsPtr->DeleteEntity(entity);
		}
		});
}

inline void AcidLMB::Update() {
	if (hasLanded) return;

	float dt = ecsPtr->m_GetDeltaTime();

	velocity.y += gravity * dt;

	auto* tf = ecsPtr->GetComponent<TransformComponent>(entity);
	if (tf) {
		tf->LocalTransformation.position += velocity * dt;

		if (glm::length(velocity) > 0.1f) {
			glm::vec3 dir = glm::normalize(velocity);
			float yaw = glm::degrees(std::atan2(dir.x, dir.z));
			float pitch = glm::degrees(std::asin(glm::clamp(-dir.y, -1.f, 1.f)));
			tf->LocalTransformation.rotation = glm::vec3(pitch, yaw, 0.f);
		}
	}

	currentTimer += dt;
	if (currentTimer >= timeBeforeDeath) {
		hasLanded = true;
		ecsPtr->DeleteEntity(entity);
	}
}

inline void AcidLMB::PlayRandomEnemyDeathSFX()
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
		std::cout << "[AcidLMB] No enemy death SFX found.\n";
		return;
	}

	int idx = rand() % static_cast<int>(enemyDeathSfxPool.size());
	enemyDeathSfxPool[idx]->requestPlay = true;

	std::cout << "[AcidLMB] Playing enemy death SFX index " << idx << "\n";
}

inline void AcidLMB::SpawnGasCloud() {
	if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

		for (auto& af : ac->audioFiles) {
			if (af.audioGUID == grenadeExplosionSfxGUID && af.isSFX) {
				af.requestPlay = true;
				break;
			}
		}
	}

	if (gasCloudPrefab == utility::GUID{}) {
		std::cout << "[AcidLMB] No gas cloud prefab assigned!\n";
		return;
	}

	auto* selfTf = ecsPtr->GetComponent<TransformComponent>(entity);
	if (!selfTf) return;

	std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
	ecs::EntityID gasCloudID = DuplicatePrefabIntoScene<R_Scene>(currentScene, gasCloudPrefab);

	if (auto* cloudTf = ecsPtr->GetComponent<TransformComponent>(gasCloudID)) {
		cloudTf->LocalTransformation.position += selfTf->WorldTransformation.position;
		std::cout << "[AcidLMB] Gas cloud spawned at: "
			<< selfTf->WorldTransformation.position.x << ", "
			<< selfTf->WorldTransformation.position.y << ", "
			<< selfTf->WorldTransformation.position.z << "\n";
	}
}