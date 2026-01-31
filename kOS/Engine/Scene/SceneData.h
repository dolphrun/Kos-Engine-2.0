#pragma once
#include "Config/pch.h"
#include "ECS/ECSList.h"

class SceneData
{
public:
	std::vector<std::string> skyBoxGUID;
	utility::GUID NavMeshGuid;
	utility::GUID postProcessingProfile{};
	float ambientIntensity{ 1.0f };
	
	
	REFLECTABLE(SceneData, skyBoxGUID, NavMeshGuid, postProcessingProfile, ambientIntensity);


public:
	bool isPrefab{ false };
	bool isActive{ true };
	std::string sceneName;
	ecs::EntityID prefabID = 0;
	std::vector<ecs::EntityID> sceneIDs;
};