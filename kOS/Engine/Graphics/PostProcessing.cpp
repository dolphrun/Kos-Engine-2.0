#include "PostProcessing.h"

Shader* Vigniette::currentShader{ nullptr };
Shader* FilmGrain::currentShader{ nullptr };
Shader* ChromaticAberration::currentShader{ nullptr };

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


ChromaticAberration::ChromaticAberration()
	: redOffset(0.0f), greenOffset(0.0f), blueOffset(0.0f)
{
}

ChromaticAberration::ChromaticAberration(const ChromaticAberration& other)
	: redOffset(other.redOffset), greenOffset(other.greenOffset), blueOffset(other.blueOffset)
{

}
void ChromaticAberration::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iRedOffset", redOffset);
	currentShader->SetFloat("iGreenOffset", greenOffset);
	currentShader->SetFloat("iBlueOffset", blueOffset);
}