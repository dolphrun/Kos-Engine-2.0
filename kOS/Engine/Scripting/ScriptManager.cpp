
/********************************************************************/
/*!
\file      ScriptManager.cpp
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
#include "ScriptManager.h"


typedef void (*DLLUpdateStatic)(StaticVariableManager*);

void ScriptManager::Init(const std::string& scriptdllPath)
{
	m_dllPath = scriptdllPath + "\\script.dll";
	RunDLL();
}

void ScriptManager::LoadDLL() {
	ScriptClass::ScriptClassGenerator.clear();
	//Load DLL from path
	hInstDLL = LoadLibraryA(m_dllPath.c_str());
	if (!hInstDLL) { LOGGING_ERROR("Failed to load DLL library"); return; }
	//Create scripts to be added

	std::string cPrefix{ "c_Create" };

	//Update systems 
	DLLUpdateStatic updateFunc = (DLLUpdateStatic)GetProcAddress(hInstDLL, "UpdateStatic");
	updateFunc(&svm);

	FreeLibrary(hInstDLL);

}
void ScriptManager::LoadScripts() {
	//Load from script path
}
void ScriptManager::ReloadDLL() {
	FreeLibrary(hInstDLL);

}


void ScriptManager::RunDLL() {
	if (!hInstDLL) {
		hInstDLL = LoadLibraryA(m_dllPath.c_str());
		if (!hInstDLL) { LOGGING_ERROR("Failed to load DLL library"); return; }
		svm.ECSSystem = &m_ecs;
		svm.field = &m_field;
		svm.input = &m_input;
		svm.scene = &m_sceneManager;
		svm.physics = &m_physics;
		svm.resource = &m_resourceManager;
		svm.scriptNames = &m_ecs.scriptList;
		svm.navMesh = &m_navmesh;
		svm.grpahics = &m_graphics;
		DLLUpdateStatic updateFunc = (DLLUpdateStatic)GetProcAddress(hInstDLL, "UpdateStatic");
		updateFunc(&svm);
		updateFunc = nullptr;
	}

}
void ScriptManager::PeformHotReload()
{
	if (hInstDLL) {
		UnloadDLL();
	}

	//


	RunDLL();

}
void AssetDLLTMP(std::string const&) { }

void ScriptManager::UnloadDLL() {
	//Loop through each script and free them
	//ScriptList.clear();

	if (hInstDLL) {
		for (auto a : ScriptClass::ScriptClassGenerator) {
			a.second = nullptr;
		}
		
		
		ScriptClass::ScriptClassGenerator.clear();
		m_field.GetAction().clear();
		//clear ecs componentpool
		for (const auto& scripts : m_ecs.scriptList) {
			m_ecs.FreeComponentPool(scripts);
			m_ecs.componentAction.erase(scripts);
			m_ecs.GetComponentsString().erase(scripts);
		}
		m_ecs.scriptList.clear();

		FreeLibrary(hInstDLL);
		hInstDLL = nullptr;
	}
	}


void ScriptManager::AddGenerator(std::string const& id) {
	//Create function name
	std::string functionName{ "c_Create" + id };
	//Add create function to the map
	LOGGING_INFO("Adding function {}", functionName);

	//Add file name to the script txt
	//For debug purposes
	GenerateScriptClass procAddress = (GenerateScriptClass)GetProcAddress(hInstDLL, functionName.c_str());
	ScriptClass::ScriptClassGenerator[id] = procAddress;
}
