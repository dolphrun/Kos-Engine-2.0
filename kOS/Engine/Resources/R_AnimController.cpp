#include "Config/pch.h"
#include "R_AnimController.h"
#include "..\DeSerialization\json_handler.h"
void R_AnimController::Load()
{
	this->m_AnimControllerData = serialization::ReadJsonFile<AnimControllerData>(this->GetFilePath().string());
	std::cout << "CONTROLLER PATH" << this->GetFilePath().string() << '\n';

	if (!this->m_AnimControllerData.states.empty())
		this->m_AnimControllerData.currentState = &this->m_AnimControllerData.states[0];
}
void R_AnimController::Unload() {}