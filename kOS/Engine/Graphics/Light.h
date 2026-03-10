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
#pragma once
#include "GraphicsReferences.h"
#include "Shader.h"

enum LightType {
	LIGHT		=0,
	DIRECTIONAL =1,
	POINTLIGHT  =2,
	SPOTLIGHT	=3,
};


struct PointLightData {

	PointLightData() = default;
	PointLightData(glm::vec3 pos, glm::vec3 col, glm::vec3 diff, glm::vec3 spec, float lin, float quad, float inten) :
		position(pos), color(col), diffuseStrength(diff), specularStrength(spec), linear(lin), quadratic(quad), intensity{ inten } {

	};
	PointLightData(glm::vec3 pos, glm::vec3 col, glm::vec3 diff, glm::vec3 spec, float lin, float quad, float inten, bool shadowCast, bool bc = false, utility::GUID bmGUID = utility::GUID{}) :
		position(pos), color(col), diffuseStrength(diff), specularStrength(spec), linear(lin), quadratic(quad), intensity{ inten }, shadowCon{ shadowCast }, bakedCon{ bc }, bakedmapGUID{ bmGUID } {
	};

	glm::vec3 position{0.f,0.f,0.f};
	glm::vec3 color{1.f,1.f,1.f};
	glm::vec3  diffuseStrength{1.f};
	glm::vec3  specularStrength{1.f};
	float intensity;
	float linear{0.09f};
	float quadratic{ 0.032f };
	bool shadowCon; 
	bool bakedCon;
	utility::GUID bakedmapGUID;
	static glm::vec3  ambientStrength;
	virtual void SetUniform(Shader* shader, size_t number);
	virtual void SetShaderMtrx(Shader* shader, size_t number);
};

struct DirectionalLightData :public PointLightData {

	DirectionalLightData() = default;
	
	DirectionalLightData(glm::vec3 pos, glm::vec3 col, glm::vec3 diff, glm::vec3 spec, float lin, float quad, float intens, glm::vec3 dir)
		: PointLightData(pos, col, diff, spec, lin, quad, intens), direction(dir), intensity(intens) {
	}

	glm::vec3 direction{ 0.f,0.f,1.f };
	float intensity;
	void SetUniform(Shader* shader, size_t number);
	void SetShaderMtrx(Shader* shader, size_t number);
	void SetShaderMtrx(Shader* shader, size_t number,const glm::mat4& viewMtx);
};

struct SpotLightData :public PointLightData {

	SpotLightData() = default;

	SpotLightData(glm::vec3 pos, glm::vec3 col, glm::vec3 diff, glm::vec3 spec, float lin, float quad, float intens, glm::vec3 dir, float cutoff, float outercutoff) :
		PointLightData(pos, col, diff, spec, lin, quad, intens), direction(dir), cutOff(cutoff), outerCutOff(outercutoff) {};

	glm::vec3 direction{0.f,0.f,1.f};
	float cutOff{5.5f};
	float outerCutOff{10.5f};
	void SetUniform(Shader* shader, size_t number);
};