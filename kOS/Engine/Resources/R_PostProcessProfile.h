#pragma once
#include "Resource.h"
#include "Config/pch.h"
#include "Graphics/PostProcessing.h"

class R_PostProcessingProfile : public Resource
{
public:
	using Resource::Resource;
	void Load() override;
	void Unload() override;
	PostProcessingProfile profile;
	REFLECTABLE(R_PostProcessingProfile);
};
