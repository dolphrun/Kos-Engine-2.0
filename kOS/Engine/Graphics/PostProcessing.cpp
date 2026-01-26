#include "PostProcessing.h"

Shader* Vigniette::currentShader{ nullptr };
void Vigniette::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iIntensity", intensity);
	currentShader->SetFloat("iExtent", extent);
	//Update resolution
	currentShader->SetVec2("iResolution", glm::vec2{1920,1080});
	//std::cout << intensity << ' ' << extent << '\n';
}