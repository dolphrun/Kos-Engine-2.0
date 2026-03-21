#pragma once
#include "Resource.h"
#include "Config/pch.h"

struct MaterialData
{

    utility::GUID diffuseMaterialGUID{};   // Path or ID for material asset
    utility::GUID specularMaterialGUID{};
    utility::GUID normalMaterialGUID{};
    utility::GUID ambientOcclusionMaterialGUID{};
    utility::GUID roughnessMaterialGUID{};
    utility::GUID emissiveMaterialGUID{};

    REFLECTABLE(MaterialData,
        diffuseMaterialGUID,
        specularMaterialGUID,
        normalMaterialGUID,
        ambientOcclusionMaterialGUID,
        roughnessMaterialGUID,
        emissiveMaterialGUID
	);
};

class R_Material : public Resource {
public:
    using Resource::Resource;

    void Load() override;
    void Unload() override;


    MaterialData md;
   
    REFLECTABLE(R_Material);

private:
};