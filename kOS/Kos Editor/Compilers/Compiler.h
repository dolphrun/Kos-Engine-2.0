#pragma once
#include "Config/pch.h"
#include "Resources/ResourceHeader.h"


struct MeshCompiler {
	std::string type = R_Model::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(MeshCompiler, path, outputExtension, inputExtensions);
};
struct TextureCompiler {
	std::string type = R_Texture::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(TextureCompiler, path, outputExtension, inputExtensions);

};
struct FontCompiler {
	std::string type = R_Font::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(TextureCompiler, path, outputExtension, inputExtensions);

};
struct SceneCompiler {
	std::string type = R_Scene::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(SceneCompiler, path, outputExtension, inputExtensions);

};

struct AudioCompiler {
	std::string type = R_Audio::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(AudioCompiler, path, outputExtension, inputExtensions);

};

struct AudioStudioCompiler {
	std::string type = R_AudioStudio::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(AudioStudioCompiler, path, outputExtension, inputExtensions);
};

struct MaterialCompiler {
	std::string type = R_Material::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(R_Material, path, outputExtension, inputExtensions);

};
struct DepthMapCubeCompiler {
	std::string type = R_DepthMapCube::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(R_DepthMapCube, path, outputExtension, inputExtensions);

};

struct AnimControllerCompiler {
	std::string type = R_AnimController::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(R_AnimController, path, outputExtension, inputExtensions);
};

struct NavMeshCompiler {
	std::string type = "R_NavMesh"; // temp cause nav mesh no need r_
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(NavMeshCompiler, path, outputExtension, inputExtensions);

};
struct PostProcessingCompiler {
	std::string type = R_PostProcessingProfile::classname();
	std::string path;
	std::string outputExtension;
	std::vector<std::string> inputExtensions;
	REFLECTABLE(PostProcessingCompiler, path, outputExtension, inputExtensions);
};
struct CompilerData {
	MeshCompiler meshCompiler;
	TextureCompiler textureCompiler;
	FontCompiler fontCompiler;
	SceneCompiler sceneCompiler;
	AudioCompiler audioCompiler;
	AudioStudioCompiler audioStudioCompiler;
	MaterialCompiler materialCompiler;
	DepthMapCubeCompiler dmcCompiler;
	AnimControllerCompiler animControllerCompiler;
	NavMeshCompiler navCompiler;
	PostProcessingCompiler postProcessingCompiler;
	REFLECTABLE(CompilerData, meshCompiler, textureCompiler, fontCompiler, sceneCompiler, audioCompiler, audioStudioCompiler, materialCompiler, dmcCompiler, animControllerCompiler, navCompiler, postProcessingCompiler);
};