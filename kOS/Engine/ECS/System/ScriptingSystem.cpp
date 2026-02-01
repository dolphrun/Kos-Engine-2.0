
#include "Config/pch.h"
#include "ECS/ECS.h"
#include "ScriptingSystem.h"
#include "Scripting/ScriptManager.h"
#include "ECS/Component/ScriptComponent.h"
#include "Resources/ResourceManager.h"
#include "Debugging/Performance.h"

namespace ecs {



	void ScriptingSystem::Init()
	{
		
		//Use The DLL
	}

	void ScriptingSystem::Update()
	{

		const auto& entities = m_entities.Data();
		
		for (const std::string& scriptName : m_ecs.scriptList) {

			std::chrono::duration<float> scriptDuration{};
			auto start = std::chrono::steady_clock::now();

			auto action = m_ecs.componentAction.at(scriptName);
			try {
				auto& entityList = m_ecs.GetComponentsEnties(scriptName);
				for (const EntityID id : entityList) {

					//TODO - find better way to go about this
					SceneData sceneData = m_ecs.GetSceneData(m_ecs.GetSceneByEntityID(id));
					auto* nc = m_ecs.GetComponent<NameComponent>(id);
					if (!sceneData.isActive || nc->hide) continue;
					
				

					auto script = static_cast<ScriptClass*>(m_ecs.GetIComponent<void*>(scriptName, id));

					if (!script->isStart) {
						script->isStart = true;
						script->Start();
						
					}

					script->Update();
					int steps = m_physicsManager.FrameCount();
					for (int i = 0; i < steps; i++)
					{
						script->FixedUpdate();
					}
				}
			}
			catch (...) {
				continue;
			}
			auto end = std::chrono::steady_clock::now();
			scriptDuration = (end - start);
			m_performance.SetScriptValue(scriptName, scriptDuration.count());
			
		}
		




	}


}
