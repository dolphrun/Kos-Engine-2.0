#pragma once
#include "Graphics/GraphicsReferences.h"
#include "Graphics/Shader.h"
struct PostProcessEffect {
	virtual Shader* GetShader()=0;
	virtual void UpdateShader() {};
};
struct Vigniette :public PostProcessEffect {
	 void UpdateShader();
	 float intensity,extent;
	 static Shader* currentShader;
	 Shader* GetShader() { return currentShader;; };
	 glm::vec2 resolution;

};
class PostProcessingProfile {
	public:
	std::string profileName;
	std::vector<std::unique_ptr<PostProcessEffect>>postProcessingEffects;
};