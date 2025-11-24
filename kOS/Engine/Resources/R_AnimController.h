#pragma once
#include "Config/pch.h"
#include "Resource.h"
#include "ResourceHeader.h"

struct AnimState;
struct AnimPin;
struct AnimTransition;

//The dots
struct AnimPin {
	int id;
	enum PinKind
	{
		Input,
		Output
	} kind;
	std::string name;
	int fromParentId{};
	int toParentId{};
	REFLECTABLE(AnimPin, id, kind, name, fromParentId, toParentId);
};

struct AnimCondition {
	std::string name{};
	bool triggered{ false };
	REFLECTABLE(AnimCondition, name, triggered);
};

//The lines
struct AnimTransition {
	
	int id;
	int fromPinId;
	int toPinId;
	AnimCondition condition;
	REFLECTABLE(AnimTransition, id, fromPinId, toPinId, condition);
};

//The boxes
struct AnimState {
	int id;
	std::vector<AnimPin> inputs;
	std::vector<AnimPin> outputs;

	std::string name{};
	utility::GUID animationGUID;
	float playSpeed = 1.0f;
	bool isLooping = true;
	bool isDefault = false;
	std::vector<AnimTransition> outgoingTransitions;

	REFLECTABLE(AnimState, id, inputs, outputs, name, animationGUID, playSpeed, isLooping, outgoingTransitions);
};


struct AnimControllerData
{ 
	std::string name{};
	std::vector<AnimState> states{};
	int nextStateID{ 1 };
	int nextPinID{ 100 };
	int nextLinkID{ 200 };

	AnimState* currentState;

	REFLECTABLE(AnimControllerData, name, states, nextStateID, nextPinID, nextLinkID);
};

class R_AnimController :public Resource
{
public:
	using Resource::Resource;
	void Load() override;
	void Unload() override;

	AnimControllerData m_AnimControllerData;

	REFLECTABLE(R_AnimController);
};