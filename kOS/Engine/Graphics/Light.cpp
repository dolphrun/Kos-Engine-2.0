/********************************************************************/
/*!
\file      Light.h
\author    Gabe Ng 2301290 Sean Tiu 2303398
\par       gabe.ng@digipen.edu s.tiu@digipen.edu
\date      Oct 03 2025
\brief     The main data types for our lights, in here we have
           - Directional
           - Point
           - Spot light

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "Light.h"


glm::vec3 PointLightData::ambientStrength{ 0.01f };

 
float CaluclateRadius(glm::vec3 color, float linear, float quadratic) {
    return (-linear + std::sqrt(linear * linear - 4 * quadratic * (1.0f - (256.0f / 5.0f) * std::fmaxf(std::fmaxf(color.r, color.g), color.b)))) / (2.0f * quadratic);;;
}
void PointLightData::SetUniform(Shader* shader,size_t number) {
	shader->Use();

    std::stringstream s;

    // Set the position of the light
    s << "light[" << number << "].position";
    shader->SetVec3(s.str(), this->position);

    s.str("");
    s << "light[" << number << "].color";
    shader->SetVec3(s.str(), this->color);

    s.str("");
    s << "light[" << number << "].La";
    shader->SetVec3(s.str(), this->ambientStrength);

    s.str("");
    s << "light[" << number << "].Ld";
    shader->SetVec3(s.str(), this->diffuseStrength);

    s.str("");
    s << "light[" << number << "].Ls";
    shader->SetVec3(s.str(), this->specularStrength);


    s.str("");
    s << "light[" << number << "].linear";
    shader->SetFloat(s.str(), this->linear);

    s.str("");
    s << "light[" << number << "].quadratic";
    shader->SetFloat(s.str(), this->quadratic);

    s.str("");
    s << "light[" << number << "].intensity";
    shader->SetFloat(s.str(), this->intensity);

    s.str("");
    s << "light[" << number << "].shadowCon";
    shader->SetBool(s.str(), this->shadowCon);
    s.str("");
    s << "light[" << number << "].bakedCon";
    shader->SetBool(s.str(), this->bakedCon);
    s.str("");
    s << "light[" << number << "].radius";
    shader->SetFloat(s.str(), CaluclateRadius(this->color,linear,quadratic));
    shader->Disuse();
}

void SpotLightData::SetUniform(Shader* shader, size_t number) {

    shader->Use();

    std::stringstream s;

    // Set the position of the light
    s << "spotLight[" << number << "].position";
    shader->SetVec3(s.str(), this->position);

    s.str("");
    s << "spotLight[" << number << "].color";
    shader->SetVec3(s.str(), this->color);

    s.str("");
    s << "spotLight[" << number << "].La";
    shader->SetVec3(s.str(), this->ambientStrength);

    s.str("");
    s << "spotLight[" << number << "].Ld";
    shader->SetVec3(s.str(), this->diffuseStrength);

    s.str("");
    s << "spotLight[" << number << "].Ls";
    shader->SetVec3(s.str(), this->specularStrength);


    s.str("");
    s << "spotLight[" << number << "].linear";
    shader->SetFloat(s.str(), this->linear);

    s.str("");
    s << "spotLight[" << number << "].quadratic";
    shader->SetFloat(s.str(), this->quadratic);

    s.str("");
    s << "spotLight[" << number << "].radius";
    shader->SetFloat(s.str(), CaluclateRadius(this->color, linear, quadratic));

    
    s.str("");
    s << "spotLight[" << number << "].direction";
    shader->SetVec3(s.str(), normalize(direction));

    s.str("");
    s << "spotLight[" << number << "].cutOff";
    shader->SetFloat(s.str(), glm::cos(glm::radians(this->cutOff)));

    s.str("");
    s << "spotLight[" << number << "].outerCutOff";
    shader->SetFloat(s.str(), glm::cos(glm::radians(this->outerCutOff)));

    s.str("");
    s << "spotLight[" << number << "].intensity";
    shader->SetFloat(s.str(), this->intensity);

    shader->Disuse();

}
void DirectionalLightData::SetUniform(Shader* shader, size_t number) {
    shader->Use();

    std::stringstream s;

    // Set the position of the light
    s << "directionalLight[" << number << "].direction";
    shader->SetVec3(s.str(), normalize(-direction));

    s.str("");
    s << "directionalLight[" << number << "].color";
    shader->SetVec3(s.str(), this->color);

    s.str("");
    s << "directionalLight[" << number << "].La";
    shader->SetVec3(s.str(), this->ambientStrength);

    s.str("");
    s << "directionalLight[" << number << "].Ld";
    shader->SetVec3(s.str(), this->diffuseStrength);

    s.str("");
    s << "directionalLight[" << number << "].Ls";
    shader->SetVec3(s.str(), this->specularStrength);

    s.str("");
    s << "directionalLight[" << number << "].intensity";
    shader->SetFloat(s.str(), this->intensity);
    shader->Disuse();
}

void PointLightData::SetShaderMtrx(Shader* shader, size_t number) {
    shader->Use();

    shader->Disuse();
}

void DirectionalLightData::SetShaderMtrx(Shader* shader, size_t number) {
    shader->Use();
    std::stringstream s;
    s << "directionalLight[" << number << "].shadowMtx";
    //Calculate shadow mtx
    float near_plane = -50.f, far_plane = 100.f;
    //Update light direction
    glm::vec3 lightPosition =  - glm::normalize(this->direction) * 100.f;
    glm::mat4 lightSpaceMatrix = glm::ortho(-40.f, 40.f, -30.f, 30.f, near_plane, far_plane) *glm::lookAt(this->direction,
                                                           glm::vec3(0.0f, 0.0f, 0.0f),
                                                              glm::vec3(0.0f, 1.0f, 0.0f));
    shader->SetTrans(s.str(), lightSpaceMatrix);

    shader->Disuse();
}