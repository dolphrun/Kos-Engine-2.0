#include "Graphics/GraphicsReferences.h"
#include "Graphics/Shader.h"
struct PostProcessEffect {
	Shader* currentShader;
	virtual void UpdateShader();
};
struct Vigniette :public PostProcessEffect {
	 void UpdateShader() {
		 //Set intensity
		 currentShader->SetFloat("Intensity",intensity);
		 currentShader->SetFloat("Extent", extent);
	 }
	 float intensity,extent;
};
class PostProcessingProfile {
	public:
	std::vector<std::unique_ptr<PostProcessEffect>>postProcessingEffects;
};