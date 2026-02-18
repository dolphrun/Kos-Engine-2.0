#include "Config/pch.h"
#include "Graphics/GraphicsManager.h"
#define STB_IMAGE_IMPLEMENTATION
#include <STB_IMAGE/stb_image.h>
#include "R_Texture.h"
void R_Texture::Load()
{	
	std::filesystem::path path = this->GetFilePath();
	if (path.extension() == ".dds")
	{
		LoadDSSTexture(this->m_filePath.string().c_str());
	}
	else
	{
		/// Might wanna handle other file cases
		stbiLoad(this->m_filePath.string().c_str());
	}
}

void R_Texture::Unload()
{
	FreeTexture();
}

void R_Texture::stbiLoad(const char* texturePath) {
	//Set texture
	glGenTextures(1, &texture);
	//Bind textures
	glBindTexture(GL_TEXTURE_2D, texture);
	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//Load texture
	int nrChannels;
	unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);
	//Load texture
	if (!data) {
		std::cout << "FAILED TO LOAD TEXURE";
		texture = Textures::missingTexture.RetrieveTexture();
	}
	///std::cout << "Channel of [" << texName << "] is " << nrChannels << std::endl;
	if (nrChannels == 3) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else if (nrChannels == 4) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else if (nrChannels == 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_RED);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_ONE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
	}
	glGenerateMipmap(GL_TEXTURE_2D);
	//Free texture
	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void R_Texture::LoadDSSTexture(const char* texturePath) {
	FreeTexture();
	gli::texture Texture = gli::load(texturePath);
	if (Texture.empty()) {
		//Use a generic texture...
		texture = Textures::missingTexture.RetrieveTexture();
		return;;
	}

	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const Format = GL.translate(Texture.format(), Texture.swizzles());
	GLenum Target = GL.translate(Texture.target());
	//assert(gli::is_compressed(Texture.format()) && Target == gli::TARGET_2D);

	glm::tvec3<GLsizei> Extent(Texture.extent(0));

	glGenTextures(1, &texture);
	glBindTexture(Target, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(Target, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(Target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(Texture.levels() - 1));
	glTexParameteriv(Target, GL_TEXTURE_SWIZZLE_RGBA, &Format.Swizzles[0]);
	glTexStorage2D(Target, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x, Extent.y);
	for (std::size_t Level = 0; Level < Texture.levels(); ++Level)
	{
		glm::tvec3<GLsizei> LevelExtent(Texture.extent(Level));
		glCompressedTexSubImage2D(
			Target, static_cast<GLint>(Level), 0, 0, LevelExtent.x, LevelExtent.y,
			Format.Internal, static_cast<GLsizei>(Texture.size(Level)), Texture.data(0, 0, Level));
	}

}

void R_Texture::FreeTexture() {
	//std::cout << "Freeing data";
	//textureList.erase(name);
	glDeleteTextures(1, &texture);
}

/************************************************************************/
/*!
\brief
Activates the texture and binds it to the current OpenGL context.

\return
NIL
*/
/************************************************************************/
void R_Texture::Use() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
}
/************************************************************************/
/*!
\brief
Returns the OpenGL texture ID.

\return
The texture ID (`GLuint`).
*/
/************************************************************************/
//Access width and height
unsigned int R_Texture::RetrieveTexture() {
	return texture;
}
/************************************************************************/
/*!
\brief
Returns the OpenGL texture ID pointer.

\return
The texture ID (`GLuint`) pointer.
*/
/************************************************************************/
//Access width and height
unsigned int* R_Texture::RetrieveTexturePointer() {
	return &texture;
}

/************************************************************************/
/*!
\brief
Retrieves the width of the texture.

\return
The width of the texture in pixels.
*/
/************************************************************************/
int R_Texture::RetrieveWidth() const {
	return width;
}
/************************************************************************/
/*!
\brief
Retrieves the height of the texture.

\return
The height of the texture in pixels.
*/
/************************************************************************/
int R_Texture::RetrieveHeight() const {
	return height;
}
/************************************************************************/
/*!
\brief
Retrieves the width and height of the texture as a pair.

\return
A `std::pair<int, int>` containing the width and height of the texture.
*/
/************************************************************************/
std::pair<int, int> R_Texture::RetrieveWH() const {
	return std::pair<int, int>{ width, height };
}
/************************************************************************/
/*!
\brief
Returns the name of the texture.

\return
The name of the texture (`std::string`).
*/
/************************************************************************/
std::string R_Texture::RetrieveName() const {
	return name;
};

R_Texture::TextureType R_Texture::RetrieveType() { return type; }

void R_Texture::SetType(TextureType typ) { type = typ; }

