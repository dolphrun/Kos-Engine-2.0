/********************************************************************/
/*!
\file      Material.cpp
\author    Gabe Ng 2301290
\par       gabe.ng@digipen.edu
\date      Oct 03 2025
\brief     Contains the material type to be used for PBR

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "Material.h"
#include <sstream>

void Material::SetUniform(Shader* shader) {
    shader->Use();

    std::stringstream s;

    // Set the position of the light
    s << "material.Ka";
    shader->SetVec3(s.str(), this->ambience);

    s.str("");
    s << "material.Kd";
    shader->SetVec3(s.str(), this->diffuse);

    s.str("");
    s << "material.Ks";
    shader->SetVec3(s.str(), this->specular);

    s.str("");
    s << "material.shininess";
    shader->SetFloat(s.str(), this->shininess);

    s.str("");
    s << "material.reflectivity";
    shader->SetFloat(s.str(), this->reflectivity);

    shader->Disuse();
}
PBRMaterial::PBRMaterial(std::shared_ptr<R_Texture> albedoTex ,
                         std::shared_ptr<R_Texture> specularTex ,
                         std::shared_ptr<R_Texture> roughnessTex ,
                         std::shared_ptr<R_Texture> aoTex ,
                         std::shared_ptr<R_Texture> normalTex,
                         std::shared_ptr<R_Texture> emissiveTex) :
    albedo{ albedoTex }, specular{ specularTex }, roughness{ roughnessTex }, ao{ aoTex }, normal{ normalTex }, emission{emissiveTex}
{}