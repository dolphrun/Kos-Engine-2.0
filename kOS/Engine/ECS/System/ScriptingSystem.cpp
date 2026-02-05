
#include "Config/pch.h"
#include "ECS/ECS.h"
#include "ScriptingSystem.h"
#include "Scripting/ScriptManager.h"
#include "ECS/Component/ScriptComponent.h"
#include "Resources/ResourceManager.h"
#include "Debugging/Performance.h"

#define SCRIPTPROFILE "Scritps"

namespace ecs {



	void ScriptingSystem::Init()
	{
		m_runScript = [&](const std::string& scriptname) {

			auto action = m_ecs.componentAction.at(scriptname);

			try {
				auto& entityList = m_ecs.GetComponentsEnties(scriptname);
				for (const EntityID id : entityList) {

					//TODO - find better way to go about this
					SceneData sceneData = m_ecs.GetSceneData(m_ecs.GetSceneByEntityID(id));
					auto* nc = m_ecs.GetComponent<NameComponent>(id);
					if (!sceneData.isActive || nc->hide) continue;



					auto script = static_cast<ScriptClass*>(m_ecs.GetIComponent<void*>(scriptname, id));

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
				return;
			}
			};
		//Use The DLL
	}

	void ScriptingSystem::Update()
	{

		const auto& entities = m_entities.Data();
		
		for (const std::string& scriptName : m_ecs.scriptList) {

			PROFILE_SYSTEM(m_performance, SCRIPTPROFILE, scriptName, m_runScript(scriptName));
		}
		




	}


}
