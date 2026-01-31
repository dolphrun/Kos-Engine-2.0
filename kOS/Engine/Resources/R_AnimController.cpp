#include "Config/pch.h"
#include "R_AnimController.h"
#include "DeSerialization\json_handler.h"
void R_AnimController::Load()
{
	this->m_AnimControllerData = serialization::ReadJsonFile<AnimControllerData>(this->GetFilePath().string());

    if (this->m_AnimControllerData.states.size() >= 3)
    {
        this->m_EnterState = &this->m_AnimControllerData.states[0];
        this->m_AnyState = &this->m_AnimControllerData.states[1];
        this->m_ExitState = &this->m_AnimControllerData.states[2];
    }
		
}
void R_AnimController::Unload() {}

AnimState* R_AnimController::FindStateFromPin(int pinId)
{
    for (auto& state : m_AnimControllerData.states)
    {
        for (auto& p : state.inputs)
            if (p.id == pinId)
                return &state;

        for (auto& p : state.outputs)
            if (p.id == pinId)
                return &state;
    }
    return nullptr;
}

AnimState* R_AnimController::RetrieveStateByID(int stateId)
{
    for (int i = 0; i < m_AnimControllerData.states.size(); i++)
    {
        AnimState* state = &m_AnimControllerData.states[i];
        if (stateId == state->id) return state;
    }
    return nullptr;
}

AnimPin* R_AnimController::FindPin(int pinId)
{
    for (auto& state : m_AnimControllerData.states)
    {
        for (auto& p : state.inputs)
            if (p.id == pinId)
                return &p;

        for (auto& p : state.outputs)
            if (p.id == pinId)
                return &p;
    }
    return nullptr;
}

AnimState* R_AnimController::RetrieveEntryState()
{
    if (!m_AnimControllerData.states.empty())
    {
        return &m_AnimControllerData.states[0];
    }

    return nullptr;
}
