/********************************************************************/
/*!
\file      UIElements.cpp
\author    Sean Tiu (2303398)
\par       Emails: s.tiu@digipen.edu
\date      Oct 03, 2025
\brief     Defines data structures and mesh classes used for
           rendering 2D UI elements such as text, sprites, and grids
           within the engine.

           This file defines several key components of the UI system:
           - **UIElementData**: Base data shared by all UI elements,
             including position, scale, rotation, and color.
           - **ScreenTextData**: Holds information for rendering
             on-screen text, such as font, string content, and
             alignment.
           - **ScreenSpriteData**: Represents 2D sprites with
             optional sprite-sheet animation support.
           - **ScreenGridData**: Defines a grid structure (currently
             unimplemented).

           It also declares mesh structures derived from `UIMesh`
           used for rendering:
           - **TextMesh**: Builds and draws text glyphs.
           - **ScreenSpriteMesh**: Renders 2D sprites and icons.
           - **GridMesh**: Reserved for rendering UI grids.

           These components work with shaders and camera data to draw
           2D user interface elements efficiently in the rendering
           pipeline.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "UIElements.h"
#include "GraphicsManager.h"

void TextMesh::CreateMesh()
{
    glGenVertexArrays(1, &this->vaoID);
    glGenBuffers(1, &this->vboID);

    glBindVertexArray(this->vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, this->vboID);

    constexpr int numberOfVertexes = 6;
    constexpr int numberOfWorldCoordinates = 2;
    constexpr int numberOfUVCoordinates = 2;
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numberOfVertexes * (numberOfWorldCoordinates + numberOfUVCoordinates), NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, (numberOfWorldCoordinates + numberOfUVCoordinates), GL_FLOAT, GL_FALSE, (numberOfWorldCoordinates + numberOfUVCoordinates) * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void TextMesh::DrawMesh(const ScreenTextData& textData, Shader& shader, const CameraData& camera)
{
    //Unimplemented for now
}

void ScreenSpriteMesh::CreateMesh()
{
    float vertices[] = {
        // pos       // tex
        -0.5f, 0.5f, 0.f,  0.0f, 1.0f, // bottom left
         -0.5f, -0.5f, 0.f,  0.0f, 0.0f, // top left
         0.5f,  0.5f, 0.f,  1.0f, 1.0f, // bottom right
         0.5f,  -0.5f, 0.f,  1.0f, 0.0f, // top right
    };
    short indices[]{
    0, 1, 2, 3
    };
    glGenVertexArrays(1, &vaoID);
    glGenBuffers(1, &vboID);
    GLuint ebo;
    glGenBuffers(1, &ebo);

    glBindVertexArray(vaoID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); // Changed to DYNAMIC_DRAW

    //Bind the element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Attribute 0 = vec3 (x, y, z)
    glVertexAttribPointer(
        0,                // location = 0 in shader
        3,                // 3 components: x, y, z
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),// stride
        (void*)0          // offset
    );
    glEnableVertexAttribArray(0);

    // Attribute 1 = vec2 (u, v)
    glVertexAttribPointer(
        1,                // location = 1 in shader
        2,                // 2 components: u, v
        GL_FLOAT,
        GL_FALSE,
        5 * sizeof(float),// stride
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ScreenSpriteMesh::DrawMesh(const ScreenSpriteData& spriteData, Shader& shader, const CameraData& camera)
{
    shader.Use();
    shader.SetFloat("uShaderType", 2.1f);
    constexpr float radianConversion = 3.14159f / 180.f;
    float angle = spriteData.rotation * radianConversion;

    shader.SetMat3("projection", camera.GetUIOrthoMtx());
    shader.SetVec3("point", spriteData.position);
    shader.SetVec2("scale", spriteData.scale);
    shader.SetFloat("rotation", angle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spriteData.textureToUse->RetrieveTexture());
    shader.SetInt("sprite", 0);
    shader.SetVec4("color", spriteData.color);
    shader.SetInt("entityID", spriteData.entityID + 1);

    glBindVertexArray(this->vaoID);

    // NEW: Update UV coordinates if custom UV is enabled
    if (spriteData.useCustomUV) {
        // Update the vertex buffer with new UV coordinates
        float vertices[] = {
            // pos                    // tex (custom UV)
            -0.5f,  0.5f, 0.f,  spriteData.uvMin.x, spriteData.uvMax.y, // bottom left
            -0.5f, -0.5f, 0.f,  spriteData.uvMin.x, spriteData.uvMin.y, // top left
             0.5f,  0.5f, 0.f,  spriteData.uvMax.x, spriteData.uvMax.y, // bottom right
             0.5f, -0.5f, 0.f,  spriteData.uvMax.x, spriteData.uvMin.y, // top right
        };

        // Bind the VBO stored in this mesh and update it
        glBindBuffer(GL_ARRAY_BUFFER, this->vboID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
        std::cout << "after OpenGL Error: " << err << std::endl;
    }
    shader.Disuse();
}

void ScreenSpriteMesh::DrawMeshWorld(const ScreenSpriteData& spriteData, Shader& shader, const CameraData& camera)
{
    shader.Use();
    shader.SetFloat("uShaderType", 2.1f);
    constexpr float radianConversion = 3.14159f / 180.f;
    float angle = spriteData.rotation * radianConversion;

    shader.SetMat4("projection", camera.GetPerspMtx());
    shader.SetMat4("view", camera.GetViewMtx());
    shader.SetVec3("point", spriteData.position);
    shader.SetVec2("scale", spriteData.scale);
    shader.SetFloat("rotation", angle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, spriteData.textureToUse->RetrieveTexture());
    shader.SetInt("sprite", 0);
    shader.SetVec4("color", spriteData.color);
    shader.SetInt("entityID", spriteData.entityID + 1);

    glBindVertexArray(this->vaoID);

    // NEW: Update UV coordinates if custom UV is enabled
    if (spriteData.useCustomUV) {
        // Update the vertex buffer with new UV coordinates
        float vertices[] = {
            // pos                    // tex (custom UV)
            -0.5f,  0.5f, 0.f,  spriteData.uvMin.x, spriteData.uvMax.y, // bottom left
            -0.5f, -0.5f, 0.f,  spriteData.uvMin.x, spriteData.uvMin.y, // top left
             0.5f,  0.5f, 0.f,  spriteData.uvMax.x, spriteData.uvMax.y, // bottom right
             0.5f, -0.5f, 0.f,  spriteData.uvMax.x, spriteData.uvMin.y, // top right
        };

        // Bind the VBO stored in this mesh and update it
        glBindBuffer(GL_ARRAY_BUFFER, this->vboID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_SHORT, NULL);
    glBindVertexArray(0);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
        std::cout << "after OpenGL Error: " << err << std::endl;
    }
    shader.Disuse();
}

void GridMesh::CreateMesh()
{
    //Unimplemented for now
}

void GridMesh::DrawMesh(const ScreenGridData& gridData, Shader& shader, const CameraData& camera)
{
    //Unimplemeted for now
}