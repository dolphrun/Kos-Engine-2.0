#include "Config/pch.h"
#include "R_AnimController.h"
#include "..\DeSerialization\json_handler.h"
void R_AnimController::Load()
{
	this->m_AnimControllerData = serialization::ReadJsonFile<AnimControllerData>(this->GetFilePath().string());
	std::cout << "CONTROLLER PATH" << this->GetFilePath().string() << '\n';
}
void R_AnimController::Unload() {}