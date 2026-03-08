/******************************************************************/
/*!
\file      Prefab.h
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Nov 11, 2024
\brief     This file the class prefabs that read an json file and 
		   stores each object into the prefabs. When the function
		   is called. The prefab, would create and clone its data
		   onto the entity


Copyright (C) 2024 DigiPen Institute of Technology. 
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef PREFAB_H
#define PREFAB_H

#include "ECS/ECSList.h"
#include "ECS/ECS.h"
#include "Scene/SceneManager.h"
#include "AssetManager/AssetManager.h"
#include "DeSerialization/json_handler.h"

namespace prefab {

	class PrefabManager {

		ecs::ECS& m_ecs;
		scenes::SceneManager& m_sceneManager;
		AssetManager& m_assetManager;
		serialization::Serialization& m_serialization;

	public:
		PrefabManager(ecs::ECS& ecs, scenes::SceneManager& sceneManager, AssetManager& am, serialization::Serialization& seri) :
			m_ecs(ecs),
			m_sceneManager(sceneManager),
			m_assetManager(am),
			m_serialization(seri)
		{
		}

		void SetPrefabStatus(ecs::EntityID id, bool status);
		void AssignPrefabToNameComponent(ecs::EntityID parentid, std::string scenename);
		int m_CreatePrefab(std::string prefabscene, std::string insertscene = {});
		void m_SaveEntitytoPrefab(ecs::EntityID);
		void OverwriteScenePrefab(ecs::EntityID id);
		void UpdateAllPrefab(const std::string& prefabSceneName);
		void DeepUpdatePrefab(ecs::EntityID idA, ecs::EntityID idB, bool updateParentTransform = false);
		void OverwritePrefab_Component(ecs::EntityID entityID, const std::string& componentName, const std::string& prefabSceneName, ecs::EntityID comparedID);
		void RevertToPrefab_Component(ecs::EntityID entityID, const std::string& componentName, ecs::EntityID comparedID);
		void LoadPrefab(const std::filesystem::path& filepath);
		void LoadAllPrefabs();
		ecs::ComponentSignature ComparePrefabWithInstance(ecs::EntityID id, ecs::EntityID compare = -1);
		void RefreshComponentDifferenceList(std::vector<std::string>& diffComp, ecs::EntityID entityID);
		void CompareAll(std::map<EntityID, std::pair<EntityID, ecs::ComponentSignature>>& result, ecs::EntityID entityID);
		void CompareEntity(std::map<EntityID, std::pair<EntityID, ecs::ComponentSignature>>& result, ecs::EntityID entityID, ecs::EntityID compare);
	};


}
#endif PREFAB_H

