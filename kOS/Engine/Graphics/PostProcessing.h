#pragma once
#include "Graphics/GraphicsReferences.h"
#include "Graphics/Shader.h"
enum PostProcessType {
	PPT_Vigniette=0,
	PPT_FilmGrain=1,
	PPT_ChromaticAbberation=2
};
struct PostProcessEffect {
	virtual Shader* GetShader()=0;
	virtual void UpdateShader() {};
	virtual PostProcessType GetType()=0;
};
struct Vigniette :public PostProcessEffect {
	Vigniette();
	Vigniette(const Vigniette& other);
	 void UpdateShader();
	 float intensity,extent;
	 static Shader* currentShader;
	 Shader* GetShader() { return currentShader;; };
	 PostProcessType GetType() { return PostProcessType::PPT_Vigniette;;};
	 glm::vec2 resolution;

};
struct FilmGrain : public PostProcessEffect {
	FilmGrain();
	FilmGrain(const FilmGrain& other);
	void UpdateShader();
	float noiseStrength;
	static Shader* currentShader;
	Shader* GetShader() { return currentShader;; };
	PostProcessType GetType() { return PostProcessType::PPT_FilmGrain;; };
};

struct ChromaticAberration : public PostProcessEffect {
	ChromaticAberration();
	ChromaticAberration(const ChromaticAberration& other);
	void UpdateShader();
	float redOffset;
	float greenOffset;
	float blueOffset;
	static Shader* currentShader;
	Shader* GetShader() { return currentShader;; };
	PostProcessType GetType() { return PostProcessType::PPT_ChromaticAbberation;;};
};

class PostProcessingProfile {
	public:
	std::string profileName;
	std::vector<std::unique_ptr<PostProcessEffect>>postProcessingEffects;
};