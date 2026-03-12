#pragma once
#include "TemplateSC.h"

class GameManager : public TemplateSC {
public:
	utility::GUID enemyObjectPrefab;

	utility::GUID enemySpawnpoint;
	ecs::EntityID enemySpawnpointID;

	utility::GUID gameBGM;
	utility::GUID ambienceBGM;

	void Start() override {
		// ADD SFX OF BGM HERE - NEED WAV BUT CODE HERE ALR
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == gameBGM && af.isBGM) {
					std::cout << "audio playing";
					af.requestPlay = true;
					break;
				}
			}

			enemySpawnpointID = ecsPtr->GetEntityIDFromGUID(enemySpawnpoint);
		}

		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == ambienceBGM && af.isBGM) {
					std::cout << "audio playing";
					af.requestPlay = true;
					break;
				}
			}
		}
	}
	void Update() override {
		//// SPAWN ENEMIES
		//if (Input->IsKeyTriggered(keys::X)) {
		//	std::shared_ptr<R_Scene> enemyObject = resource->GetResource<R_Scene>(enemyObjectPrefab);

		//	if (enemyObject) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID enemyObjectID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyObjectPrefab);

		//		if (auto* enemyObjectTransform = ecsPtr->GetComponent<TransformComponent>(enemyObjectID)) {
		//			enemyObjectTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(enemySpawnpointID)->WorldTransformation.position;
		//		}
		//	}
		//}
	}

	REFLECTABLE(GameManager, enemyObjectPrefab, enemySpawnpoint, gameBGM, ambienceBGM)
};