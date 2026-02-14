#ifndef R_TEXTUREH
#define R_TEXTUREH

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Config/pch.h"
#include "Resource.h"
#include <stdlib.h>
#include <gli/gl.hpp>
#include <gli/gli.hpp>

class R_Texture : public Resource {

	enum TextureType {
		REGULAR = 0,
		DIFFUSE = 1,
		SPECULAR = 2,
		NORMAL = 3,
		HEIGHT = 4,
		ROUGHNESS
	};
	enum FileType {
		WIC = 0,
		DSS = 1
	};

public:
	using Resource::Resource;
	void Load() override;

	void Unload() override;

	void Use();
	unsigned int RetrieveTexture();
	unsigned int* RetrieveTexturePointer();
	int RetrieveWidth() const;
	int RetrieveHeight() const;
	std::pair<int, int> RetrieveWH() const;
	std::string RetrieveName() const;
	TextureType RetrieveType();
	void SetType(TextureType typ);
	unsigned int GetTextureID() {
		return texture;
	}

	REFLECTABLE(R_Texture);

private:
	unsigned int texture{};
	int width{}, height{};
	std::string name{};
	TextureType type{ TextureType::REGULAR };

	void stbiLoad(const char* texturePath);
	void LoadDSSTexture(const char* texturePath);
	void FreeTexture();
	
	
};

#endif // ! R_TEXTUREH

