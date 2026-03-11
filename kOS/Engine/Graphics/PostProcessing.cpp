#include "PostProcessing.h"

Shader* Vigniette::currentShader{ nullptr };
Shader* FilmGrain::currentShader{ nullptr };
Shader* ChromaticAberration::currentShader{ nullptr };
Shader* Blur::currentShader{ nullptr };

glm::vec2 PostProcessEffect::screenResolution{ 1920.f,1080.f };
Vigniette::Vigniette()
	: intensity(0.0f),
	extent(0.0f),
	resolution(0.0f, 0.0f),
	color(0.f,0.f,0.f)
{
}

Vigniette::Vigniette(const Vigniette& other)
	: intensity(other.intensity),
	extent(other.extent),
	resolution(other.resolution),
	color(other.color)
{
}
void Vigniette::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iIntensity", intensity);
	currentShader->SetVec2("iResolution", this->screenResolution);
	currentShader->SetFloat("iExtent", extent);
	//Update resolution
	currentShader->SetVec3("iVignetteColor", this->color);
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


Blur::Blur()
	: radius(0.0f), axis(0)
{
}

Blur::Blur(const Blur& other)
	: radius(other.radius), axis(0)
{

}
void Blur::UpdateShader() {
	//Set intensity
	currentShader->SetFloat("iRadius", radius);
	float val = this->screenResolution.x;
	if (axis)val = screenResolution.y;
	currentShader->SetVec2("iResoVal",screenResolution);

}


Bloom::Bloom()
	: filterRadius(0.0f)
{
}

Bloom::Bloom(const Bloom& other)
	: filterRadius(other.filterRadius)
{

}
void Bloom::UpdateShader() {


}