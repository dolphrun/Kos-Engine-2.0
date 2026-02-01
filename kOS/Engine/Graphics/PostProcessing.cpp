#include "PostProcessing.h"

Shader* Vigniette::currentShader{ nullptr };
Shader* FilmGrain::currentShader{ nullptr };
Vigniette::Vigniette()
	: intensity(0.0f),
	extent(0.0f),
	resolution(0.0f, 0.0f)
{
}

Vigniette::Vigniette(const Vigniette& other)
	: intensity(other.intensity),
	extent(other.extent),
	resolution(other.resolution)
{

}
void Vigniette::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iIntensity", intensity);
	currentShader->SetFloat("iExtent", extent);
	//Update resolution
	currentShader->SetVec2("iResolution", glm::vec2{1920,1080});
	//std::cout << intensity << ' ' << extent << '\n';
}

FilmGrain::FilmGrain()
	: noiseStrength(0.0f)
{
}

FilmGrain::FilmGrain(const FilmGrain& other)
	: noiseStrength(other.noiseStrength)
{

}
void FilmGrain::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iNoise", noiseStrength);
	//std::cout << intensity << ' ' << extent << '\n';
}
