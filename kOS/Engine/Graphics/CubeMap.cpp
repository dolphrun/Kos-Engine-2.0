/********************************************************************/
/*!
\file      CubeMap.cpp
\author    Gabe Ng 2301290
\par       gabe.ng@digipen.edu
\date      Oct 03 2025
\brief     This file houses the cube map class, used for construction of
           - The skybox
           - The Shadow map
           - The irradiance map

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "CubeMap.h"

//Load a cubemap

void CubeMap::InitializeMap(std::vector<std::string> texFiles) {
    LoadCubeModel();
    LoadCubeTexture(texFiles);
}
void CubeMap::LoadCubeTexture(std::vector<std::string> faces) {

    // Generate a texture ID for the cube map
    glGenTextures(1, &texID);

    // Bind the texture as a cube map
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    GLuint i = 0;
    // Iterate through the provided file names
    for (std::string filename : faces)
    {
        GLint w, h; // Variables to store the width and height of the texture

        // Load the HDR image data for the current file
        float* data = stbi_loadf(filename.c_str(), &w, &h, NULL, 3);

        if (!data) // Throw an exception if the file cannot be loaded
            throw std::runtime_error(std::string("file ") + filename + " not found.");

        if (i == 0) // Allocate immutable storage for the whole cube map texture
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB32F, w, h);
        // Upload the image data to the appropriate cube map face
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);

        // Free the image data after uploading it to the GPU
        stbi_image_free(data);

        i++;
    }

    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



}

void CubeMap::LoadCubeTextureDDS(std::vector<std::string> faces) {
    //std::cout << "LAODING CUBE MAP DDS\n";
    
    // Generate texture ID
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    gli::gl GL(gli::gl::PROFILE_GL33);
    gli::texture Texture = gli::load(faces[0]); // Use first face to get format & size
    if (Texture.format() == gli::FORMAT_UNDEFINED)    std::cout << "BAD JUJU\n"<<faces[0];
    gli::gl::format Format = GL.translate(Texture.format(), Texture.swizzles());
    glm::tvec3<GLsizei> Extent(Texture.extent(0));

    // Allocate storage once for all 6 faces
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, static_cast<GLint>(Texture.levels()), Format.Internal, Extent.x, Extent.y);

    // Upload each face
    for (GLuint i = 0; i < faces.size(); ++i) {
        gli::texture FaceTex = gli::load(faces[i]);

        for (std::size_t level = 0; level < FaceTex.levels(); ++level) {
            glm::tvec3<GLsizei> LevelExtent(FaceTex.extent(level));
            glCompressedTexSubImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                static_cast<GLint>(level),
                0, 0,
                LevelExtent.x, LevelExtent.y,
                Format.Internal,
                static_cast<GLsizei>(FaceTex.size(level)),
                FaceTex.data(0, 0, level)
            );
        }
    }

    // Set texture filtering parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}
void CubeMap::LoadCubeModel() {
    float cubeMapVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // skybox VAO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeMapVertices), &cubeMapVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

}
int CubeMap::RetrieveID() { return texID;; }
void Skybox::Render(Shader* shader, glm::mat4 const& view, glm::mat4 const& projection) {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    shader->Use();
    shader->SetTrans("view", view);
    shader->SetTrans("projection", projection);
    // skybox cube
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    shader->Disuse();

}

void IrradianceMap::InitializeMap() {
    
    LoadCubeModel();
    //Set up irradiance texture
   glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    for (unsigned int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
void IrradianceMap::Render(Shader* irradianceShader,Skybox sb) {
    irradianceShader->Use();
    irradianceShader->SetInt("environmentMap", 0);
    irradianceShader->SetTrans("projection", captureProjection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, sb.RetrieveID());

    for (unsigned int i = 0; i < 6; ++i)
    {
        irradianceShader->SetTrans("view", captureViews[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, texID, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

    }

    irradianceShader->Disuse();

}
void IrradianceMap::RenderCube(Shader* shader, glm::mat4 const& view, glm::mat4 const& projection) {
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_FALSE);
    shader->Use();
    shader->SetTrans("view", view);
    shader->SetTrans("projection", projection);
    // skybox cube
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    shader->Disuse();
}

void DepthCubeMap::InitializeMap() {

    //Generate cube map fbo
    glGenFramebuffers(1, &this->VBO);

    //Initialize depth cube map
    glGenTextures(1, &this->texID);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->texID);
    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    //SEt texture parameters
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->VBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->texID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}
void DepthCubeMap::FillMap(glm::vec3& lightPos) {

    glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), (float)1024.f / (float)1024.f, near_plane, far_plane);
    shadowTransforms[0] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[1] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[2] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)));
    shadowTransforms[3] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)));
    shadowTransforms[4] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
    shadowTransforms[5] = (shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f)));
}

void DepthCubeMap::SaveDepthCubeMap(std::string outputPath) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);
    //Save depth cube map to the specified output path
    std::ofstream file(outputPath, std::ios::binary);
    for (int i = 0; i < 6; ++i)
    {
        std::vector<unsigned char> data(1024 * 1024 * 4); // 4 for RGBA
        glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data.data());
        file.write(reinterpret_cast<char*>(data.data()), data.size());
    }

    file.close();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

}

void DepthCubeMap::LoadDepthCubeMap(std::string inputPath) {
    std::ifstream file(inputPath, std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: Could not open file for writing: " << inputPath << std::endl;
        return;
    }
    //Generate cube map fbo
    glGenFramebuffers(1, &this->VBO);

    //Initialize depth cube map
    glGenTextures(1, &this->texID);
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    glBindTexture(GL_TEXTURE_CUBE_MAP, this->texID);
    for (int i = 0; i < 6; ++i)
    {
        std::vector<unsigned char> data(SHADOW_WIDTH * SHADOW_HEIGHT * 4);
        file.read(reinterpret_cast<char*>(data.data()), data.size());
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
            SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data.data());
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, this->VBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, this->texID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //std::cout << "LOADED DEPTH MAP DATA\n";
    file.close();
}
