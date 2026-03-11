#pragma once

/********************************************************************/
/*!
\file      ScriptManager.h
\author    Gabe Ng 2301290
\par       gabe.ng@digipen.edu
\date      Oct 03 2025
\brief     Handles the loading of DLL and manipulation of DLL elements.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "Script.h"

#include "ECS/ECS.h"
#include "Inputs/Input.h"
#include "Physics/PhysicsManager.h"
#include "Resources/ResourceManager.h"
#include "Reflection/Field.h"
#include "Scene/SceneManager.h"
#include "Pathfinding/NavMesh.h"

namespace scenes {
	class SceneManager;
}

class NavMeshManager;

struct StaticVariableManager {
	void* ECSSystem;
	void* field;
	void* input;
	void* scene;
	void* physics;
	void* resource;
	void* navMesh;
	void* grpahics;
	std::vector<std::string>* scriptNames;
};

class ScriptManager {
	ecs::ECS& m_ecs;
	Input::InputSystem& m_input;
	scenes::SceneManager& m_sceneManager;
	physics::PhysicsManager& m_physics;
	ResourceManager& m_resourceManager;
	Fields& m_field;
	NavMeshManager& m_navmesh;
	GraphicsManager& m_graphics;


public: 
	ScriptManager(
		ecs::ECS& ecs,
		scenes::SceneManager& sm,
		Input::InputSystem& slm,
		physics::PhysicsManager& pm,
		ResourceManager& rm,
		Fields& field,
		NavMeshManager& nm,
		GraphicsManager& graphics
	)
		: m_ecs(ecs)
		, m_input(slm)
		, m_sceneManager(sm)
		, m_physics(pm)
		, m_resourceManager(rm)
		, m_field(field)
		, m_navmesh(nm)
		, m_graphics(graphics)
	{
		hInstDLL = nullptr;
	}


	void Init(const std::string& scriptdllPath);


	void LoadDLL();
	void LoadScripts();
	void ReloadDLL();
	void RunDLL();
	void PeformHotReload();
	void UnloadDLL();
   
	void AddGenerator(std::string const& id);

	
private:
	
	std::string m_dllPath;
	HINSTANCE hInstDLL;
	StaticVariableManager svm{};
};
