
#include "Config/pch.h"
#include "ECS.h"
#include "Component/ComponentHeader.h"
#include "Debugging/Logging.h"
#include "Reflection/ReflectionInvoker.h"
#include "Reflection/Field.h"
#include "Scene/SceneManager.h"


//ECS Varaible

namespace ecs{
	std::unordered_map<std::string, std::function<std::shared_ptr<IActionInvoker>()>> ComponentTypeRegistry::actionFactories;

	void ECS::Load() {

		//Allocate memory to each component pool
		RegisterComponent<NameComponent>();
		RegisterComponent<TransformComponent>();
		RegisterComponent<SpriteComponent>();
		RegisterComponent<CameraComponent>();
		RegisterComponent<AudioComponent>();
		RegisterComponent<AudioListenerComponent>();
		RegisterComponent<TextComponent>();
		RegisterComponent<MeshFilterComponent>();
		RegisterComponent<CanvasRendererComponent>();
		RegisterComponent<MaterialComponent>();
		RegisterComponent<SkinnedMeshRendererComponent>();
		RegisterComponent<AnimatorComponent>();
		RegisterComponent<LightComponent>();
		RegisterComponent<RigidbodyComponent>();
		RegisterComponent<BoxColliderComponent>();
		RegisterComponent<CapsuleColliderComponent>();
		RegisterComponent<SphereColliderComponent>();
		RegisterComponent<OctreeGeneratorComponent>();
		RegisterComponent<PathfinderComponent>();
		RegisterComponent<PathfinderTargetComponent>();
		RegisterComponent<CubeRendererComponent>();
		RegisterComponent<SphereRendererComponent>();
		RegisterComponent<ButtonComponent>();
		RegisterComponent<ParticleComponent>();
		RegisterComponent<NavMeshComponent>();
		RegisterComponent<VideoComponent>();

		//Allocate memory to each system

		//run in series
		RegisterSystem<ScriptingSystem>(0, RUNNING, WAIT);
		RegisterSystem<TransformSystem, TransformComponent>(0);
		RegisterSystem<BoxColliderSystem, TransformComponent, BoxColliderComponent>(0);
		RegisterSystem<CapsuleColliderSystem, TransformComponent, CapsuleColliderComponent>(0);
		RegisterSystem<SphereColliderSystem, TransformComponent, SphereColliderComponent>(0);
		RegisterSystem<RigidbodySystem, TransformComponent, RigidbodyComponent>(0, RUNNING);
		RegisterSystem<PhysicsSystem, TransformComponent, RigidbodyComponent>(0, RUNNING);



		//Render Level 3
		RegisterSystem<CameraSystem, TransformComponent, CameraComponent>(0);
		RegisterSystem<MeshRenderSystem, TransformComponent, MaterialComponent, MeshFilterComponent>(0);
		RegisterSystem<SkinnedMeshRenderSystem, TransformComponent, SkinnedMeshRendererComponent, AnimatorComponent>(0);
		RegisterSystem<CubeRenderSystem, TransformComponent, MaterialComponent, CubeRendererComponent>(0);
		RegisterSystem<SphereRenderSystem, TransformComponent, MaterialComponent, SphereRendererComponent>(0);
		RegisterSystem<CanvasTextRenderSystem, TransformComponent, CanvasRendererComponent>(0);
		RegisterSystem<CanvasSpriteRenderSystem, TransformComponent, CanvasRendererComponent>(0);
		RegisterSystem<AnimatorSystem, TransformComponent, AnimatorComponent>(0, RUNNING);
		RegisterSystem<CanvasButtonRenderSystem, TransformComponent, CanvasRendererComponent>(0);
		RegisterSystem<AnimatorSystem, TransformComponent, AnimatorComponent>(0);
		RegisterSystem<LightingSystem, TransformComponent, LightComponent>(0);
		RegisterSystem<DebugBoxColliderRenderSystem, TransformComponent, BoxColliderComponent>(0);
		RegisterSystem<DebugCapsuleColliderRenderSystem, TransformComponent, CapsuleColliderComponent>(0);
		RegisterSystem<DebugSphereColliderRenderSystem, TransformComponent, SphereColliderComponent>(0);
		RegisterSystem<AudioListenerSystem, TransformComponent, AudioListenerComponent>(0);
		RegisterSystem<AudioSystem, TransformComponent, AudioComponent>(0);
		RegisterSystem<ParticleSystem, TransformComponent, ParticleComponent>(0);
		RegisterSystem<VideoSystem, TransformComponent, VideoComponent>(0);


	}

	void ECS::Init() {
		//loops through all the system
		for (auto& System : m_systemList) {
			System.ptr->Init();
		}


	}


	void ECS::Update(float DT) {

		//update deltatime
		
		m_deltaTime = DT;
		
		//check for gamestate
		if (m_state != m_nextState) {
			if (m_nextState == START) {
				m_state = RUNNING;
				m_nextState = RUNNING;
			}
			else {
				m_state = m_nextState;
			}

				
		}
		
		RunSystemsInSeries(m_systemMap.at(0));		
		
	}

	void ECS::EndFrame() {
		if (m_deletedEntities.size() > 0) {
			std::sort(m_deletedEntities.begin(), m_deletedEntities.end());
			m_deletedEntities.erase(
				std::unique(m_deletedEntities.begin(), m_deletedEntities.end()),
				m_deletedEntities.end()
			);

			//clear and deletedEntities
			for (EntityID id : m_deletedEntities) {
				DeleteEntityImmediate(id);
			}
			m_deletedEntities.clear();
		}
	}

	void ECS::Unload() {
		m_combinedComponentPool.clear();
	}

	void ECS::RegisterEntity(EntityID ID) {

		for (auto& system : m_systemList) {
			if ((m_entityMap.find(ID)->second & system.ptr->GetSignature()) == system.ptr->GetSignature()) {

				system.ptr->RegisterSystem(ID);

			}
		}
	}

	void ECS::DeregisterEntity(EntityID ID) {

		for (auto& system : m_systemList) {
			if ((m_entityMap.find(ID)->second & system.ptr->GetSignature()) == system.ptr->GetSignature()) {

				system.ptr->DeregisterSystem(ID);

			}
		}

	}

	EntityID ECS::CreateEntity(const std::string& scene) {

		EntityID ID = 0;
		if (m_entityCount < MaxEntity) {
			ID = m_entityCount++;
		}
		else {
			ID = m_availableEntityID.front();
			m_availableEntityID.pop_front();
		}

		

		// set bitflag to 0
		m_entityMap[ID] = 0;


		//assign entity to scenes
		if(sceneMap.find(scene) == sceneMap.end()){
			LOGGING_WARN("Scene does not exits");
			throw std::runtime_error("Scene does not exits");
		}

		sceneMap.find(scene)->second.sceneIDs.push_back(ID);

		//add transform component and name component as default
		AddComponent<NameComponent>(ID);
		AddComponent<TransformComponent>(ID);

		return ID;
	}

	EntityID ECS::DuplicateEntity(EntityID DuplicatesID, std::string scene) {

		if (scene.empty()) {
			const auto& result = GetSceneByEntityID(DuplicatesID);
			if (result.empty()) {
				LOGGING_ERROR("Scene does not exits");
			}
			scene = result;
		}
		else {
			if (sceneMap.find(scene) == sceneMap.end()) {
				LOGGING_ERROR("Scene does not exits");
			}
		}

		EntityID NewEntity = CreateEntity(scene);
		ComponentSignature DuplicateSignature = m_entityMap.find(DuplicatesID)->second;

		for (const auto& [ComponentName, key] : m_componentKey) {
			if (DuplicateSignature.test(key)) {
				auto& action = componentAction.at(ComponentName);
				auto* comp = action->DuplicateComponent(DuplicatesID, NewEntity);
			}
		}

		m_entityMap.find(NewEntity)->second = DuplicateSignature;
		RegisterEntity(NewEntity);
		//checks if duplicates entity has parent and assign it
		if (GetParent(DuplicatesID).has_value()) {
			//transform->m_childID.push_back(NewEntity);
			auto parent = GetParent(DuplicatesID).value();
			SetParent(parent, NewEntity);
		}

		//checks if entity has child call recursion
		if (GetChild(DuplicatesID).has_value()) {
			//clear child id of vector for new entity
			TransformComponent* transform = GetComponent<TransformComponent>(NewEntity);
			transform->m_childID.clear();

			std::vector<EntityID> childID = GetChild(DuplicatesID).value();
			for (const auto& child : childID) {
				EntityID dupChild = DuplicateEntity(child, scene);
				SetParent(NewEntity, dupChild);
			}
		}

		return NewEntity;

	}


	void ECS::DeleteEntity(EntityID ID) {
		//check if id is a thing
		if (m_entityMap.find(ID) == m_entityMap.end()) {
			LOGGING_ERROR("Entity Does Not Exist");
			return;
		}


		m_deletedEntities.emplace_back(ID);
	}

	void ECS::DeleteEntityImmediate(EntityID ID) {

		
		//check if id is a thing
		if (m_entityMap.find(ID) == m_entityMap.end()) {
			LOGGING_ERROR("Entity Does Not Exist");
			return;
		}


		if (GetParent(ID).has_value()) {
			EntityID parent = GetParent(ID).value();
			// if parent id is deleted, no need to remove its child
			if (m_entityMap.find(parent) != m_entityMap.end()) {
				TransformComponent* parentTransform = GetComponent<TransformComponent>(parent);
				size_t pos{};
				for (EntityID& id : parentTransform->m_childID) {
					if (ID == id) {
						parentTransform->m_childID.erase(parentTransform->m_childID.begin() + pos);
						break;
					}
					pos++;
				}
			}
		}


		DeregisterEntity(ID);



		// remove entity from scene
		const auto& result = GetSceneByEntityID(ID);
		if (!result.empty()) {
			auto& entityList = sceneMap.find(result)->second.sceneIDs;
			auto it = std::find(entityList.begin(), entityList.end(), ID);
			sceneMap.find(result)->second.sceneIDs.erase(it);
		}

		//get child
		if (GetChild(ID).has_value()) {
			std::vector<EntityID> childs = GetChild(ID).value();
			for (auto& x : childs) {
				DeleteEntityImmediate(x);
			}
		}

		//delete guid off map
		utility::GUID guid = GetComponent<NameComponent>(ID)->entityGUID;
		if (!guid.Empty() && GetEntityIDFromGUID(guid) == ID) {
			
			DeleteGUID(guid);
		}

		

		// reset all components
		for (const auto& [ComponentName, key] : m_componentKey) {
			if (m_entityMap.find(ID)->second.test(key)) {
				m_combinedComponentPool[ComponentName]->Delete(ID);
			}
		}

		//delete stored entity
		m_entityMap.erase(ID);		
		m_availableEntityID.push_back(ID);

		return;
	}

	



	void ECS::FreeComponentPool(const std::string& componentName) {
		if (m_combinedComponentPool.find(componentName) != m_combinedComponentPool.end()) {
			m_combinedComponentPool.erase(componentName);
		}

	}

	const std::pmr::vector<EntityID>& ECS::GetComponentsEnties(const std::string& componentName) {
		if (m_combinedComponentPool.find(componentName) != m_combinedComponentPool.end()) {
			return m_combinedComponentPool.at(componentName)->GetEntityList();
		}
		else {
			throw std::runtime_error("ecs get component entities");
		}
	}

	std::string ECS::GetSceneByEntityID(ecs::EntityID entityID) {
		for (const auto& [sceneName, entityList] : sceneMap) {
			// Check if the entityID is in the current vector of entity IDs
			if (std::find(entityList.sceneIDs.begin(), entityList.sceneIDs.end(), entityID) != entityList.sceneIDs.end()) {
				return sceneName;  // Found the matching scene name
			}
		}
		return std::string();  // No match found
	}


	void ECS::SetParent(EntityID parent, EntityID child, bool updateTransform) {


		RemoveParent(child);

		TransformComponent* parentTransform = GetComponent<TransformComponent>(parent);
		//checks if child is already in parent
		if (GetParent(child).has_value()) {
			return;
		}

		//checks if parent is getting dragged into its child
		EntityID id = parent;
		while (GetParent(id).has_value()) {
			EntityID checkParentid = GetParent(id).value();
			if (checkParentid == child) {
				LOGGING_WARN("Cannot assign parent to its own child");
				return;
			}
			id = checkParentid;

		}

		parentTransform->m_childID.push_back(child);

		TransformComponent* childTransform = GetComponent<TransformComponent>(child);
		childTransform->m_haveParent = true;
		childTransform->m_parentID = parent;
		// Recalculate Local Transform after parenting
		if (updateTransform) {
			childTransform->localTransform = glm::inverse(parentTransform->transformation) * childTransform->transformation;
			utility::DecomposeMtxIntoTRS(childTransform->localTransform, childTransform->LocalTransformation.position, childTransform->LocalTransformation.rotation, childTransform->LocalTransformation.scale);
		}
	}

	void ECS::RemoveParent(EntityID child, bool updateTransform) {
		// removes id from both the child and the parents vector

		if (!GetParent(child).has_value()) {
			// does not have parrent
			return;
		}

		EntityID parent = GetParent(child).value();
		TransformComponent* parentTransform = GetComponent<TransformComponent>(parent);
		size_t pos{};
		for (EntityID& id : parentTransform->m_childID) {
			if (child == id) {
				parentTransform->m_childID.erase(parentTransform->m_childID.begin() + pos);
				break;
			}
			pos++;
		}

		TransformComponent* childTransform = GetComponent<TransformComponent>(child);
		childTransform->m_haveParent = false;
		childTransform->m_parentID = -1;

		

		std::function<void(EntityID)> UpdatePrefabStatus = [&](EntityID id)
		{
			NameComponent* nc = GetComponent<NameComponent>(id);
			if (nc->isPrefab) {
				nc->isPrefab = false;
				TransformComponent* tc = GetComponent<TransformComponent>(child);

				auto childIds = GetChild(id);

				if (childIds.has_value()) {
					for (EntityID id : childIds.value()) {
						UpdatePrefabStatus(id);
					}
				}

			}
			


		};
		
		UpdatePrefabStatus(child);
		

		



		//Updating Transformation Mtxs
		if (updateTransform) {
			childTransform->localTransform = childTransform->transformation;
			utility::DecomposeMtxIntoTRS(childTransform->localTransform, childTransform->LocalTransformation.position, childTransform->LocalTransformation.rotation, childTransform->LocalTransformation.scale);
		}
	}

	std::optional<EntityID> ECS::GetParent(EntityID child)
	{
		TransformComponent* childTransform = GetComponent<TransformComponent>(child);
		if (!childTransform || !childTransform->m_haveParent) {
			return std::optional<EntityID>();
		}

		return childTransform->m_parentID;

	}

	std::optional<std::vector<EntityID>> ECS::GetChild(EntityID parent)
	{
		TransformComponent* parentTransform = GetComponent<TransformComponent>(parent);
		if (parentTransform->m_childID.size() <= 0) {
			return std::optional<std::vector<EntityID>>();
		}

		return parentTransform->m_childID;

	}

	void ECS::SetActive(EntityID ID, bool active) {

		ecs::NameComponent* nc = GetComponent<ecs::NameComponent>(ID);
		ecs::TransformComponent* tc =GetComponent<ecs::TransformComponent>(ID);
		nc->hide = !active;

		if (active) {
			RegisterEntity(ID);
		}
		else {
			DeregisterEntity(ID);
		}

		if (tc->m_childID.size() > 0) {
			for (auto child_id : tc->m_childID) {
				SetActive(child_id, active);
			}

		}
	}

	void ECS::RunSystemsInParallel(const std::vector<SystemData>& systems) {
		for (auto& system : systems) {
			if (system.ptr->TestState(m_state)) {
				m_threadPool.Enqueue([this, &system]() {

					std::chrono::duration<float> systemDuration{};
					auto start = std::chrono::steady_clock::now();

					system.ptr->Update();

					auto end = std::chrono::steady_clock::now();
					systemDuration = (end - start);
					m_performance.SetSystemValue(system.systemName, systemDuration.count());
					
					});
				}
			}


		m_threadPool.Wait(); // hard sync point for ECS frame
	}

	void ECS::RunSystemsInSeries(const std::vector<SystemData>& systems) {
		for (auto& system : systems) {
			if (system.ptr->TestState(m_state)) {

				PROFILE_SYSTEM(m_performance, system.systemName, system.ptr->Update());
			}
		}
	}

}



