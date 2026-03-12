/********************************************************************/
/*!
\file      Renderer.cpp
\author    Sean Tiu (2303398)
\par       Emails: s.tiu@digipen.edu
\date      Oct 03, 2025
\brief     Defines various renderer structures used in the graphics
		   system, each responsible for rendering a specific type of
		   object within the scene.

		   This file defines several rendering modules that inherit
		   from the BasicRenderer interface:
		   - MeshRenderer: Renders 3D meshes.
		   - TextRenderer: Handles on-screen text rendering.
		   - SpriteRenderer: Draws 2D sprites and UI elements.
		   - LightRenderer: Renders different types of scene lights
			 (point, directional, and spot).
		   - DebugRenderer: Visualizes debug primitives such as
			 cubes, frustums, and light gizmos.

		   Each renderer maintains its own render data and provides
		   initialization, rendering, and clearing functions used
		   during the graphics pipeline.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "Renderer.h"
#include "Resources/R_Font.h"
#include "Resources/R_Model.h"

void TextRenderer::InitializeTextRendererMeshes()
{
	screenTextMesh.CreateMesh();
}

void TextRenderer::RenderScreenFonts(const CameraData& camera, Shader& shader)
{

	if (!screenTextToDraw.empty() && camera.size.y != 0 && camera.size.x != 0) // Need to check for camera size
	{
		for (ScreenTextData& textData : screenTextToDraw)
		{
			//if (!textData.fontToUse)continue;;
			// Activate corresponding render state
			Shader& fontShader{ shader };
			fontShader.Use();
			glm::vec3 point = textData.position;
			constexpr float radianConversion = 3.1451f / 180.f;
			float angle = textData.rotation * radianConversion;

			glm::mat3 rotationMatrix = {
			cos(angle), -sin(angle), 0.0f,
			sin(angle), cos(angle),  0.0f,
			0.0f,       0.0f,       1.0 };

			fontShader.SetMat3("projection", camera.GetUIOrthoMtx()); // Orthographic Projection
			fontShader.SetMat3("rotate", rotationMatrix);
			fontShader.SetVec3("point", point);
			fontShader.SetVec4("textColor", textData.color);

			glActiveTexture(GL_TEXTURE0);
			glBindVertexArray(screenTextMesh.vaoID);
			float originalXPosition{ textData.position.x };
			float originX{ textData.position.x };
			float originY{ textData.position.y };

			constexpr float loadedFontSize = 48.f;
			const float textXScalar = textData.scale.x * textData.fontSize / loadedFontSize;
			const float textYScalar = textData.scale.y * textData.fontSize / loadedFontSize;

			if (textData.isCentered)
			{
				// Step 1: Calculate total width and height of the text
				float totalWidth = 0.0f;
				float maxAscent = 0.0f;
				float maxDescent = 0.0f;
				for (int i = 0; i < textData.textToDraw.size(); ++i) // To calculate the total width of the text
				{
					char c = textData.textToDraw[i];
					R_Font::CharacterData ch = textData.fontToUse->m_characters[c];
					if (i == textData.textToDraw.size() - 1) // If it's the last letter, add only the letter size
					{
						totalWidth += ((ch.m_size.x * textXScalar) / ((static_cast<float>(camera.size.y))));
					}
					else // Add the letter size and the space
					{
						totalWidth += ((ch.m_advance) * textXScalar) / ((static_cast<float>(camera.size.y)));
					}
					maxAscent = std::max(maxAscent, (ch.m_bearing.y * textYScalar) / ((static_cast<float>(camera.size.y))));
					maxDescent = std::max(maxDescent, ((ch.m_size.y - ch.m_bearing.y) * textYScalar) / ((static_cast<float>(camera.size.y))));
				}
				float totalHeight = maxAscent + maxDescent;

				// Adjust starting position to center the text
				textData.position.x = originX - totalWidth / 2.0f;  // Horizontal centering
				textData.position.y = originY + maxAscent / 2.0f - totalHeight / 2.0f;  // Vertical centering
			}

			else
			{
				if (!textData.textToDraw.size())return;
				//std::cout << textData.fontToUse->m_characters.size()<< '\n';
				//float xpos = (fonts["SF-Pro.ttf"][textData.textToDraw[0]].m_bearing.x / ((static_cast<float>(camera.size.y))) * (textXScalar));
				float xpos = (textData.fontToUse->m_characters[textData.textToDraw[0]].m_bearing.x
					/ ((static_cast<float>(camera.size.y))) * (textXScalar));
				textData.position.x -= xpos;
			}


			// Step 2: Render the text
			for (const char& c : textData.textToDraw)
			{
				R_Font::CharacterData ch = textData.fontToUse->m_characters[c];
				// Calculate position and size for each character quad
				float xpos = (textData.position.x + ch.m_bearing.x / ((static_cast<float>(camera.size.y))) * (textXScalar));
				float ypos = (textData.position.y - ((float)ch.m_size.y - (float)ch.m_bearing.y) / ((static_cast<float>(camera.size.y))) * (textYScalar));
				float w = ch.m_size.x * textXScalar / ((static_cast<float>(camera.size.y)));
				float h = ch.m_size.y * textYScalar / ((static_cast<float>(camera.size.y)));

				// Update VBO for each character with texture coordinates from the atlas
				float vertices[6][4] =
				{
					{ xpos,     ypos + h,   ch.m_topLeftTexCoords.x, ch.m_topLeftTexCoords.y },
					{ xpos,     ypos,       ch.m_topLeftTexCoords.x, ch.m_bottomRightTexCoords.y },
					{ xpos + w, ypos,       ch.m_bottomRightTexCoords.x, ch.m_bottomRightTexCoords.y },

					{ xpos,     ypos + h,   ch.m_topLeftTexCoords.x, ch.m_topLeftTexCoords.y },
					{ xpos + w, ypos,       ch.m_bottomRightTexCoords.x, ch.m_bottomRightTexCoords.y },
					{ xpos + w, ypos + h,   ch.m_bottomRightTexCoords.x, ch.m_topLeftTexCoords.y }
				};

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				// Bind the texture atlas (once for all characters)
				glBindTexture(GL_TEXTURE_2D, ch.m_textureID);

				// Update the content of the VBO memory
				glBindBuffer(GL_ARRAY_BUFFER, screenTextMesh.vboID);
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
				glBindBuffer(GL_ARRAY_BUFFER, 0);

				// Render quad
				glDrawArrays(GL_TRIANGLES, 0, 6);

				glDisable(GL_BLEND);

				// Advance 
				//  for next glyph
				textData.position.x += ((ch.m_advance) * textXScalar) / ((static_cast<float>(camera.size.y)));
			}
			textData.position.x = originalXPosition;

			// Unbind for safety
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
}

void TextRenderer::Clear()
{
	screenTextToDraw.clear();
}
//Frustim
bool InFrustum(glm::vec3 center, glm::vec3 extents, utility::Fustrum const& fust) {
	for (int i = 0; i < 6; i++) {
		utility::Plane const& plane = fust.planes[i];
		float dist = glm::dot(plane.normal, center) - plane.distance;
		float radius = glm::dot(extents, glm::abs(plane.normal));
		// If completely behind this plane, cull it
		if (dist + radius < 0) {
			return false;
		}
	}
	return true;
}
void MeshRenderer::Render(const CameraData& camera, Shader& shader)
{
	shader.SetBool("isRigged", false);
	shader.SetVec3("color", glm::vec3{1.f,1.f,1.f});
	for (std::vector<MeshData>& meshData : meshesToDraw) {
		for (MeshData& mesh : meshData)
		{
			//Do NOT feed any data in if its out of the camera fustrum
			//Compute model
			glm::vec3 extents = mesh.meshToUse->boundingBox.extents;
			glm::vec3 worldCenter = glm::vec3(mesh.transformation * glm::vec4(mesh.meshToUse->boundingBox.center, 1.0));
			glm::vec3 worldExtents = glm::vec3(
				abs(mesh.transformation[0].x) * extents.x +
				abs(mesh.transformation[1].x) * extents.y +
				abs(mesh.transformation[2].x) * extents.z,

				abs(mesh.transformation[0].y) * extents.x +
				abs(mesh.transformation[1].y) * extents.y +
				abs(mesh.transformation[2].y) * extents.z,

				abs(mesh.transformation[0].z) * extents.x +
				abs(mesh.transformation[1].z) * extents.y +
				abs(mesh.transformation[2].z) * extents.z
			);


			//use new center and extents to compute fustrun
			if (!InFrustum(worldCenter, worldExtents, camera.viewFrustum)) {
	/*			std::cout << "TEST TEST\n";
				std::cout << worldCenter.x << ' ' << worldCenter.y << ' ' << worldCenter.z<<'\n';
				std::cout << extents.x << ' ' << extents.y << ' ' << extents.z << '\n';*/
			
				continue;;
			}

			shader.SetTrans("model", mesh.transformation);
			shader.SetInt("entityID", mesh.entityID + 1);
			mesh.meshToUse->PBRDraw(shader, mesh.meshMaterial);
		}
	}


}
void MeshRenderer::Render(const CameraData& camera, Shader& shader, layer::LAYERS layer)
{
	shader.SetBool("isRigged", false);
	shader.SetVec3("color", glm::vec3{ 1.f,1.f,1.f });
		for (MeshData& mesh : meshesToDraw[layer])
		{
			//Use camera culling
			shader.SetTrans("model", mesh.transformation);
			shader.SetInt("entityID", mesh.entityID + 1);
			mesh.meshToUse->PBRDraw(shader, mesh.meshMaterial);
		}
	


}
void SkinnedMeshRenderer::Update() {
	for (std::vector<SkinnedMeshData>& meshData : skinnedMeshesToDraw) {
		for (SkinnedMeshData& mesh : meshData)
		{
			/*if (mesh.animationToUse) {
				size_t boneCount = mesh.meshToUse->GetBoneInfo().size();
				mesh.finalBoneMatrices.resize(boneCount, glm::mat4(1.0f));
				mesh.animationToUse->Update(mesh.currentDuration, glm::mat4(1.f), glm::mat4(1.f), mesh.meshToUse->GetBoneMap(), mesh.meshToUse->GetBoneInfo(), mesh.finalBoneMatrices);
			}*/
			if (!mesh.animationToUse) continue;

			size_t boneCount = mesh.meshToUse->GetBoneInfo().size();
			mesh.finalBoneMatrices.resize(boneCount, glm::mat4(1.0f));

			// Base animation — unchanged from your original
			mesh.animationToUse->Update(
				mesh.currentDuration, glm::mat4(1.f), glm::mat4(1.f),
				mesh.meshToUse->GetBoneMap(),
				mesh.meshToUse->GetBoneInfo(),
				mesh.finalBoneMatrices);

			// No overlay active
			if (!mesh.overlayAnimation || mesh.overlayWeight <= 0.f) continue;

			std::vector<glm::mat4> overlayMatrices(boneCount, glm::mat4(1.0f));
			mesh.overlayAnimation->Update(
				mesh.overlayTime, glm::mat4(1.f), glm::mat4(1.f),
				mesh.meshToUse->GetBoneMap(),
				mesh.meshToUse->GetBoneInfo(),
				overlayMatrices);

			// Build mask flags once before the blend loop
			const auto& boneMap = mesh.meshToUse->GetBoneMap();
			bool hasMask = !mesh.overlayBoneMask.empty();
			std::vector<bool> boneInMask(boneCount, true);

			if (hasMask)
			{
				// Default all bones to excluded
				std::fill(boneInMask.begin(), boneInMask.end(), false);

				// Only mark bones that are in the mask list
				for (const std::string& boneName : mesh.overlayBoneMask)
				{
					auto it = boneMap.find(boneName);
					if (it != boneMap.end() && it->second < boneCount)
						boneInMask[it->second] = true;
				}
			}

			float base = 1.0f - mesh.overlayWeight;
			for (size_t i = 0; i < boneCount; i++)
			{
				if (!boneInMask[i]) continue;  // skip bones not in mask

				mesh.finalBoneMatrices[i] = (mesh.finalBoneMatrices[i] * base)
					+ (overlayMatrices[i] * mesh.overlayWeight);
			}
		}
	}
}


void SkinnedMeshRenderer::Render(const CameraData& camera, Shader& shader)
{
	shader.SetInt("texture_diffuse1", 0);
	shader.SetInt("texture_specular1", 1);
	shader.SetInt("texture_normal1", 2);
	shader.SetInt("texture_ao1", 4);
	shader.SetInt("texture_roughness1", 5);
	shader.SetBool("isRigged", true);
	shader.SetVec3("color", glm::vec3{ 1.f,1.f,1.f });
	for (std::vector<SkinnedMeshData>& meshData : skinnedMeshesToDraw) {
		for (SkinnedMeshData& mesh : meshData)
		{
			shader.SetTrans("model", mesh.transformation);
			shader.SetInt("entityID", mesh.entityID + 1);
			if (mesh.animationToUse)
			{
				shader.SetBool("isRigged", true);
				mesh.meshToUse->DrawAnimation(shader, mesh.meshMaterial, mesh.finalBoneMatrices);
			}
			else
			{
				shader.SetBool("isRigged", false);
				mesh.meshToUse->PBRDraw(shader, mesh.meshMaterial);
			}

		}
	}
}

void SkinnedMeshRenderer::Render(const CameraData& camera, Shader& shader, layer::LAYERS layer)
{
	///Might wanna check on this if this gives bugs
	shader.SetBool("isRigged", false);
	shader.SetVec3("color", glm::vec3{ 1.f,1.f,1.f });
	for (SkinnedMeshData& mesh : skinnedMeshesToDraw[layer])
	{
		shader.SetTrans("model", mesh.transformation);
		shader.SetInt("entityID", mesh.entityID + 1);
		if (mesh.animationToUse)
		{
			shader.SetBool("isRigged", true);
			mesh.meshToUse->DrawAnimation(shader, mesh.meshMaterial, mesh.finalBoneMatrices);
		}
		else
		{
			shader.SetBool("isRigged", false);
			mesh.meshToUse->PBRDraw(shader, mesh.meshMaterial);
		}
	}
}

void LightRenderer::InitializeLightRenderer() {
	for (int i{ 0 }; i < 16; i++) {
		dcm[i].InitializeMap();
	}
	//testDCM.LoadDepthCubeMap("D:/CJJJ2/kOS/Kos Editor/Assets/DepthMap/test.dcm");

	LOGGING_INFO("Initialized shadow maps\n");
}
void LightRenderer::UpdateDCM() {
	for (size_t i = 0; i < pointLightsToDraw.size(); i++)
	{
		PointLightData& pointLight = pointLightsToDraw[i];
		if (pointLight.shadowCon) {
			dcm[i].FillMap(pointLight.position);
		}
	}
}
void LightRenderer::RenderAllLights(const CameraData& camera, Shader& shader)
{
	for (size_t i = 0; i < pointLightsToDraw.size(); i++)
	{
		PointLightData& pointLight = pointLightsToDraw[i];
		if (pointLight.shadowCon) {
			//FIll up with uniform data
		}
		pointLight.SetUniform(&shader, i);
		//pointLight.SetShaderMtrx(&shader, i);

	}

	for (size_t i = 0; i < directionLightsToDraw.size(); i++)
	{
		DirectionalLightData& directionLight = directionLightsToDraw[i];
		directionLight.SetShaderMtrx(&shader, i,camera.GetViewMtx());
		directionLight.SetUniform(&shader, i);

	}

	for (size_t i = 0; i < spotLightsToDraw.size(); i++)
	{
		SpotLightData& spotLight = spotLightsToDraw[i];
		//spotLight.SetShaderMtrx(&shader, i);
		spotLight.SetUniform(&shader, i);
	}
	//std::cout << "D LIGHT SIZE" << directionLightsToDraw.size() << '\n';
}
void LightRenderer::DebugRender(const CameraData& camera, Shader& shader) {
	for (size_t i = 0; i < pointLightsToDraw.size(); i++)
	{
		PointLightData& pointLight = pointLightsToDraw[i];
	}
}

void MeshRenderer::Clear()
{
	for (std::vector<MeshData>& md : meshesToDraw) {
		md.clear();
	}
}

void SkinnedMeshRenderer::Clear()
{
	for (std::vector<SkinnedMeshData>& md : skinnedMeshesToDraw) {
		md.clear();
	}
}
void CubeRenderer::Render(const CameraData& camera, Shader& shader, Cube* cubePtr) {
	shader.SetInt("texture_diffuse1", 0);
	shader.SetInt("texture_specular1", 1);
	shader.SetInt("texture_normal1", 2);
	shader.SetInt("texture_ao1", 4);
	shader.SetInt("texture_roughness1", 5);

	shader.SetBool("isRigged", false);
	for (CubeData& cd : cubesToDraw) {
		shader.SetTrans("model", cd.transformation);
		shader.SetVec3("color", glm::vec3{ 1.f,1.f,1.f });

		shader.SetInt("entityID", cd.entityID + 1);
		glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
		
		unsigned int currentTexture = 0;
		currentTexture = (cd.meshMaterial.albedo) ? cd.meshMaterial.albedo->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind sepcular
		glActiveTexture(GL_TEXTURE1); // activate proper texture unit before binding
		
		currentTexture = (cd.meshMaterial.specular) ? cd.meshMaterial.specular->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind normal
		glActiveTexture(GL_TEXTURE2); // activate proper texture unit before binding
		
		currentTexture = (cd.meshMaterial.normal) ? cd.meshMaterial.normal->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind Metallic map
		glActiveTexture(GL_TEXTURE4); // activate proper texture unit before binding
		
		currentTexture = (cd.meshMaterial.ao) ? cd.meshMaterial.ao->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind roughness
		glActiveTexture(GL_TEXTURE5); // activate proper texture unit before binding
		
		currentTexture = (cd.meshMaterial.roughness) ? cd.meshMaterial.roughness->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//std::cout << "RENDERING MESH\n";
		glActiveTexture(GL_TEXTURE0);
		cubePtr->DrawMesh();
	}
}
void CubeRenderer::Clear() {
	cubesToDraw.clear();
}

void SphereRenderer::Render(const CameraData& camera, Shader& shader, Sphere* spherePtr) {
	shader.SetInt("texture_diffuse1", 0);
	shader.SetInt("texture_specular1", 1);
	shader.SetInt("texture_normal1", 2);
	shader.SetInt("texture_ao1", 4);
	shader.SetInt("texture_roughness1", 5);
	shader.SetBool("isRigged", false);
	for (SphereData& cd : spheresToDraw) {
		//std::cout << "RENDERING SPHERE\n";
		shader.SetTrans("model", cd.transformation);
		shader.SetVec3("color", glm::vec3{ 0.f,1.f,0.f });
		shader.SetInt("entityID", cd.entityID + 1);
		glActiveTexture(GL_TEXTURE0); // activate proper texture unit before binding
		unsigned int currentTexture = 0;
		currentTexture = (cd.meshMaterial.albedo) ? cd.meshMaterial.albedo->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind sepcular
		glActiveTexture(GL_TEXTURE1); // activate proper texture unit before binding
		currentTexture = (cd.meshMaterial.specular) ? cd.meshMaterial.specular->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind normal
		glActiveTexture(GL_TEXTURE2); // activate proper texture unit before binding
		currentTexture = (cd.meshMaterial.normal) ? cd.meshMaterial.normal->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind Metallic map
		glActiveTexture(GL_TEXTURE4); // activate proper texture unit before binding
		currentTexture = (cd.meshMaterial.ao) ? cd.meshMaterial.ao->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//Bind roughness
		glActiveTexture(GL_TEXTURE5); // activate proper texture unit before binding
		currentTexture = (cd.meshMaterial.roughness) ? cd.meshMaterial.roughness->RetrieveTexture() : 0;
		glBindTexture(GL_TEXTURE_2D, currentTexture);
		//std::cout << "RENDERING MESH\n";
		glActiveTexture(GL_TEXTURE0);
		spherePtr->DrawMesh();
	}
}
void SphereRenderer::Clear() {
	spheresToDraw.clear();
}
void SpriteRenderer::InitializeSpriteRendererMeshes()
{
	screenSpriteMesh.CreateMesh();
}

void SpriteRenderer::RenderScreenSprites(const CameraData& camera, Shader& shader)
{	//Set buffer to write to first
	//Important to not remove, if not chibaboom 

	for (const ScreenSpriteData& screenSprite : screenSpritesToDraw)
	{
		screenSpriteMesh.DrawMesh(screenSprite, shader, camera);
	}
}

void SpriteRenderer::RenderWorldSprites(const CameraData& camera, Shader& shader) {
	//shader.SetInt("gMaterial", 1);
	for (const ScreenSpriteData& screenSprite : worldSpriteToDraw)
	{
		screenSpriteMesh.DrawMeshWorld(screenSprite, shader, camera);
	}

}

void SpriteRenderer::Clear()
{
	screenSpritesToDraw.clear();
	worldSpriteToDraw.clear();
}

void LightRenderer::Clear()
{
	pointLightsToDraw.clear();
	spotLightsToDraw.clear();
	directionLightsToDraw.clear();
}

void DebugRenderer::InitializeDebugRendererMeshes() {
	debugCircle.CreateMesh();
	debugFrustum.CreateMesh();
	debugCircle.CreateMesh();
	debugCube.CreateMesh();
	debugCapsule.CreateMesh();
	debugMesh.CreateMesh();
	debugLine.CreateMesh();
}
void DebugRenderer::Render(const CameraData& camera, Shader& shader) {

}
//Replace model with sphere positional data later
void DebugRenderer::RenderPointLightDebug(const CameraData& camera, Shader& shader, std::vector<PointLightData> pointLights) {
	for (size_t i = 0; i < pointLights.size(); i++)
	{
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 trY = glm::mat4(1.0f), trX = glm::mat4(1.0f);
		model = glm::translate(model, pointLights[i].position) * glm::scale(model, glm::vec3{ 1.f });
		trY = model * glm::rotate(trY, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		trX = model * glm::rotate(trX, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

		float distance = glm::length(camera.position - pointLights[i].position);
		float l = glm::sqrt(glm::pow(distance, 2.f) - glm::pow(20.f, 2.f));
		float radiusPrime = (l * 1.f) / distance;
		float t = glm::sqrt(glm::pow(1.f, (float)2) -
			glm::pow(radiusPrime, (float)2));
		t = t / distance;

		glm::vec3 centerPrime{ pointLights[i].position + t * (camera.position - pointLights[i].position) };
		glm::mat4 trS = glm::mat4(1.0f);
		trS = glm::translate(trS, centerPrime) * glm::scale(trS, glm::vec3(radiusPrime, radiusPrime, radiusPrime)) * DebugCircle::RotateZtoV(camera.position - pointLights[i].position);

		shader.SetTrans("model", model);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", glm::vec3{ 0.0f, 1.0f, 0.0f });
		debugCircle.DrawMesh();
		shader.SetTrans("model", trY);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", glm::vec3{ 0.0f, 1.0f, 0.0f });
		debugCircle.DrawMesh();
		shader.SetTrans("model", trX);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", glm::vec3{ 0.0f, 1.0f, 0.0f });
		debugCircle.DrawMesh();
		shader.SetTrans("model", trS);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", glm::vec3{ 0.0f, 1.0f, 0.0f });
		debugCircle.DrawMesh();
	}

}
void DebugRenderer::RenderDebugFrustums(const CameraData& camera, Shader& shader, const std::vector<CameraData>& debugFrustums)
{
	shader.Use();
	for (const CameraData& cam : debugFrustums)
	{
		glm::mat4 proj = cam.GetPerspMtx();   // or OrthoMtx()
		glm::mat4 view = cam.GetViewMtx();
		glm::mat4 invVP = glm::inverse(proj * view);

		glm::vec3 ndc[8] =
		{
			{-1,-1,-1}, { 1,-1,-1}, { 1, 1,-1}, {-1, 1,-1}, // near
			{-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1}, {-1, 1, 1}  // far
		};

		std::array<glm::vec3, 8> corners;
		for (int i = 0; i < 8; i++)
		{
			glm::vec4 w = invVP * glm::vec4(ndc[i], 1.0f);
			w /= w.w;
			corners[i] = glm::vec3(w);
		}
		shader.SetTrans("model", glm::mat4{ 1.f });
		shader.SetVec3("color", glm::vec3{ 0.f,1.f,0.f });           
		shader.SetMat4("vp", camera.GetViewMtx());
		shader.SetFloat("uShaderType", 2.1f);

		glBindBuffer(GL_ARRAY_BUFFER, debugFrustum.vboId);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3) * 8, corners.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		debugFrustum.DrawMesh();
	}
}

void DebugRenderer::RenderDebugCubes(const CameraData& camera, Shader& shader)
{
	//glm::mat4 model = glm::mat4(1.0f);
	//model = glm::translate(model, { 0.f,0.f,0.f }) * glm::scale(model, glm::vec3(20.f, 20.f, 20.f));	// it's a bit too big for our scene, so scale it down
	//static float dt = 0.f;
	//dt += 0.01f;
	////Draw debug object to test
	//model = glm::mat4(1.0f);
	//glm::vec3 pos = glm::vec3(0.f, 0.f, 0.f);
	//glm::quat rot = glm::vec3(glm::radians(0.f), glm::radians(0.f), glm::radians(0.f));
	//glm::vec3 sca = glm::vec3(20.f, 20.f, 20.f);
	//model = glm::translate(model, pos) * glm::mat4_cast(rot) * glm::scale(model, sca);

	for (size_t i = 0; i < basicDebugCubes.size(); i++)
	{
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetTrans("model", basicDebugCubes[i].worldTransform);
		shader.SetVec3("color", basicDebugCubes[i].color);
		debugCube.DrawMesh();
	}
}

void DebugRenderer::RenderDebugSpheres(const CameraData& camera, Shader& shader) {
	for (size_t i = 0; i < basicDebugSpheres.size(); ++i) {
		glm::vec3 pos = glm::vec3{ basicDebugSpheres[i].worldTransform[3] };
		float radius = glm::length(glm::vec3{ basicDebugSpheres[i].worldTransform[0] });
		glm::mat4 model{ 1.0f };
		model = glm::translate(model, pos) * glm::scale(model, glm::vec3{ radius });
		shader.SetTrans("model", model);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", basicDebugSpheres[i].color);
		debugCircle.DrawMesh();
		glm::mat4 trY = model * glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3{ 0.0f, 1.0f, 0.0f });
		shader.SetTrans("model", trY);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", basicDebugSpheres[i].color);
		debugCircle.DrawMesh();
		glm::mat4 trX = model * glm::rotate(glm::mat4{ 1.0f }, glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		shader.SetTrans("model", trX);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", basicDebugSpheres[i].color);
		debugCircle.DrawMesh();
		glm::vec3 viewDirection = camera.position - pos;
		glm::mat4 trS = glm::translate(glm::mat4{ 1.0f }, pos) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ radius }) * DebugCircle::RotateZtoV(viewDirection);
		shader.SetTrans("model", trS);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", basicDebugSpheres[i].color);
		debugCircle.DrawMesh();
	}
}

void DebugRenderer::RenderDebugCapsules(const CameraData& camera, Shader& shader) {
	static float lastRadius = -1.0f;
	static float lastHeight = -1.0f;
	const float cEpsilon = 1e-5f;
	for (size_t i = 0; i < basicDebugCapsules.size(); i++) {
		const auto& c = basicDebugCapsules[i];
		if (fabs(lastRadius - c.radius) > cEpsilon || fabs(lastHeight - c.height) > cEpsilon) {
			debugCapsule.DeleteMesh();
			lastRadius = c.radius;
			lastHeight = c.height;
			debugCapsule.radius = c.radius;
			debugCapsule.height = c.height;
			debugCapsule.CreateMesh();
		}
		shader.SetTrans("model", c.worldTransform);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", c.color);
		debugCapsule.DrawMesh();
		glBindVertexArray(0);
	}
}

void DebugRenderer::RenderDebugMeshes(const CameraData& camera, Shader& shader) {
	for (size_t i = 0; i < basicDebugMeshes.size(); i++) {
		const auto& data = basicDebugMeshes[i];
		debugMesh.vertices = data.vertices;
		debugMesh.indices = data.indices;
		debugMesh.CreateMesh();
		shader.SetTrans("model", data.transform);
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", data.color);
		debugMesh.DrawMesh();
		debugMesh.DeleteMesh();
	}
}

void DebugRenderer::RenderDebugLines(const CameraData& camera, Shader& shader) {
	for (const auto& line : basicDebugLines) {
		shader.SetTrans("model", glm::mat4{ 1.0f });
		shader.SetFloat("uShaderType", 2.1f);
		shader.SetVec3("color", line.color);
		debugLine.SetPosition(line.start, line.end);
		debugLine.DrawMesh();
	}
}

void DebugRenderer::Clear() {
	basicDebugCubes.clear();
	basicDebugSpheres.clear();
	basicDebugCapsules.clear();
	basicDebugMeshes.clear();
	basicDebugLines.clear();
}

void ParticleRenderer::InitializeParticleRendererMeshes()
{
	// Quad vertex data (triangle strip)
	static const float quadVertices[] = {
		//  X,     Y,    Z
		-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		 0.5f,  0.5f, 0.0f
	};

	// Texture coordinates matching triangle strip layout
	static const float quadUVs[] = {
		// U, V
		0.0f, 0.0f,   // bottom-left
		1.0f, 0.0f,   // bottom-right
		0.0f, 1.0f,   // top-left
		1.0f, 1.0f    // top-right
	};

	GLuint quadVBO;
	GLuint uvVBO;   // NEW: UV buffer

	glGenVertexArrays(1, &basicParticleMesh.vaoid);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &uvVBO);                        // NEW
	glGenBuffers(1, &basicParticleMesh.vboid);

	glBindVertexArray(basicParticleMesh.vaoid);

	//---------------------------------------------------------
	// Position buffer (attribute 0)
	//---------------------------------------------------------
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	//---------------------------------------------------------
	// UV buffer (attribute 1)
	//---------------------------------------------------------
	glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadUVs), quadUVs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	//---------------------------------------------------------
	// Instance buffer
	//---------------------------------------------------------
	glBindBuffer(GL_ARRAY_BUFFER, basicParticleMesh.vboid);

	constexpr int MAX_PARTICLES = 10000;
	glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(BasicParticleInstance), nullptr, GL_DYNAMIC_DRAW);

	GLsizei stride = sizeof(BasicParticleInstance);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BasicParticleInstance, position));

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BasicParticleInstance, scale));

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BasicParticleInstance, color));

	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BasicParticleInstance, rotation));

	glEnableVertexAttribArray(6);
	glVertexAttribIPointer(
		6,                                  // location of attribute
		1,                                  // number of components (1 int)
		GL_INT,                             // type
		stride,
		(void*)offsetof(BasicParticleInstance, textureID)
	);

	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 1, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(BasicParticleInstance, particleType));

	// Per instance divisor
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);

}

void ParticleRenderer::Render(const CameraData& camera, Shader& shader, TrailRenderer& trailRenderer)
{
	static std::vector<int> textureIDs{};
	static std::unordered_map<int, GLuint> storedIDs; //subscript, texture ID

	if (!particlesToDraw.empty())
	{
		textureIDs.clear();
		storedIDs.clear();


		instancedBasicParticles.reserve(std::accumulate(particlesToDraw.begin(), particlesToDraw.end(), size_t(0),
			[](size_t sum, const BasicParticleData& p) { return sum + p.particlePositions.size(); }));

		for (int i = 0; i < particlesToDraw.size(); ++i)
		{
			BasicParticleData& p = particlesToDraw[i];
			std::transform(p.particlePositions.begin(), p.particlePositions.end(),
				std::back_inserter(instancedBasicParticles),
				[&, j = 0](const glm::vec3& pos) mutable {
					if (p.texture_IDs != nullptr) {
						if (storedIDs.contains(p.texture_IDs->RetrieveTexture()))
						{
							return BasicParticleInstance{ pos, p.sizes[j], p.colors[j], p.rotates[j++], storedIDs[p.texture_IDs->RetrieveTexture()], p.particleType};
						}
						else
						{
							int size = static_cast<int>(textureIDs.size());
							storedIDs[p.texture_IDs->RetrieveTexture()] = static_cast<unsigned int>(textureIDs.size());
							textureIDs.push_back(p.texture_IDs->RetrieveTexture());
							int currentID = storedIDs[p.texture_IDs->RetrieveTexture()];
							return BasicParticleInstance{ pos, p.sizes[j], p.colors[j], p.rotates[j++], storedIDs[p.texture_IDs->RetrieveTexture()] , p.particleType};
						}
					}
					else {
						return BasicParticleInstance{ pos, p.sizes[j], p.colors[j], p.rotates[j++], 200,  p.particleType}; //Magic Number 200 for default particles
					}
				});
		}
		particlesToDraw.clear();
		storedIDs.clear();
	}
	shader.Use();
	shader.SetFloat("uShaderType", 2.1f);
	shader.SetTrans("projection", camera.GetPerspMtx());
	shader.SetTrans("view", camera.GetViewMtx());
	if (!instancedBasicParticles.empty())
	{
		GLenum err = glGetError();
		if (err != GL_NO_ERROR) {
			//LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
			std::cout << "before OpenGL Error: " << err << std::endl;
		}
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);  
		for (int i = 0; i < textureIDs.size(); i++)
		{
			glActiveTexture(GL_TEXTURE0 + i);
			GLint tex = textureIDs[i];
			glBindTexture(GL_TEXTURE_2D, tex);
		}

		int units[32];
		for (int i = 0; i < textureIDs.size(); i++)
			units[i] = i; // 0..N-1 texture unit indices


		glUniform1iv(glGetUniformLocation(shader.ID, "textures"),
			static_cast<GLsizei>(textureIDs.size()),
			units);

		glBindVertexArray(basicParticleMesh.vaoid);
		glBindBuffer(GL_ARRAY_BUFFER, basicParticleMesh.vboid);


		GLint boundBuffer = 0;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &boundBuffer);
		///std::cout << "Bound VBO ID: " << boundBuffer
		///	<< " expected: " << basicParticleMesh.vboid << std::endl;

		err = glGetError();
		if (err != GL_NO_ERROR) {
			//LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
		std::cout << "after 1 OpenGL Error: " << err << std::endl;
		}
		glBufferSubData(GL_ARRAY_BUFFER, 0, instancedBasicParticles.size() * sizeof(BasicParticleInstance), instancedBasicParticles.data());
		err = glGetError();
		if (err != GL_NO_ERROR) {
			//LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
			std::cout << "after 2 OpenGL Error: " << err << std::endl;
		}

		glEnable(GL_BLEND);
		glDepthMask(GL_FALSE);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(instancedBasicParticles.size()));
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		err = glGetError();
		if (err != GL_NO_ERROR) {
			//LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
			std::cout << "after 3 OpenGL Error: " << err << std::endl;
		}

		glEnable(GL_CULL_FACE);
	}
	shader.Disuse();
		
}

void ParticleRenderer::Clear()
{
	instancedBasicParticles.clear();
	particlesToDraw.clear();
}

void VideoRenderer::InitializeVideoRendererMeshes() {
	m_videoMesh.CreateMesh();
}

void VideoRenderer::Update(Shader& shader) {
	for (VideoData data : vecVideoData) {
		data.video->DecodeAndUpdateVideo(shader.ID, data.pause);
	}
}

void VideoRenderer::Render(const CameraData& camera, Shader& shader) {

	glUseProgram(shader.ID);
	glUniform1i(glGetUniformLocation(shader.ID, "yTexture"), 0); // Bind to texture unit 0
	glUniform1i(glGetUniformLocation(shader.ID, "uTexture"), 1); // Bind to texture unit 1
	glUniform1i(glGetUniformLocation(shader.ID, "vTexture"), 2); // Bind to texture unit 2
	
	for (VideoData data : vecVideoData) {
		
		auto videoPtr = data.video;
		//set uniform
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "transformation"), 1, GL_FALSE, glm::value_ptr(data.transformation));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "view"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMtx()));
		glUniformMatrix4fv(glGetUniformLocation(shader.ID, "projection"), 1, GL_FALSE, glm::value_ptr(camera.GetPerspMtx()));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, videoPtr->yTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, videoPtr->uTexture);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, videoPtr->vTexture);


		m_videoMesh.DrawMesh();

	}
	glUseProgram(0);
}

void VideoRenderer::Clear() {
	vecVideoData.clear();
}

void TrailRenderer::InitTrailRendererMeshes()
{
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	// Vertex layout: pos(3) color(4)
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		7 * sizeof(float),
		(void*)0
	);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		1, 4, GL_FLOAT, GL_FALSE,
		7 * sizeof(float),
		(void*)(3 * sizeof(float))
	);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

//void TrailRenderer::BuildTrailMesh(
//	const std::vector<BasicTrailData>& trails,
//	const glm::vec3& cameraPos)
//{
//	vertexBuffer.clear();
//	indexBuffer.clear();
//
//	unsigned int indexOffset = 0;
//
//	for (const BasicTrailData& trail : trails)
//	{
//		if (trail.points.size() < 2)
//			continue;
//
//		for (size_t i = 0; i < trail.points.size() - 1; i++)
//		{
//			const auto& p0 = trail.points[i];
//			const auto& p1 = trail.points[i + 1];
//
//			const float l0 = trail.lifetimes[i];
//			const float l1 = trail.lifetimes[i + 1];
//
//			glm::vec3 dir = glm::normalize(p1 - p0);
//			glm::vec3 toCamera = glm::normalize(cameraPos - p0);
//			glm::vec3 side = glm::normalize(glm::cross(dir, toCamera));
//
//			float t0 = l0 / trail.maxLifetime;
//			float t1 = l1 / trail.maxLifetime;
//
//			float w0 = trail.width * (1.0f - t0);
//			float w1 = trail.width * (1.0f - t1);
//
//			glm::vec3 p0L = p0 - side * w0 * 0.5f;
//			glm::vec3 p0R = p0 + side * w0 * 0.5f;
//			glm::vec3 p1L = p1 - side * w1 * 0.5f;
//			glm::vec3 p1R = p1 + side * w1 * 0.5f;
//
//			auto pushVertex = [&](glm::vec3 pos, float alpha)
//				{
//					vertexBuffer.insert(vertexBuffer.end(),
//						{
//							pos.x, pos.y, pos.z,
//							trail.color.r,
//							trail.color.g,
//							trail.color.b,
//							alpha
//						});
//				};
//
//			float alpha0 = 1.0f - t0;
//			float alpha1 = 1.0f - t1;
//
//			pushVertex(p0L, alpha0);
//			pushVertex(p0R, alpha0);
//			pushVertex(p1L, alpha1);
//			pushVertex(p1R, alpha1);
//
//			indexBuffer.insert(indexBuffer.end(),
//				{
//					indexOffset + 0,
//					indexOffset + 2,
//					indexOffset + 3,
//
//					indexOffset + 0,
//					indexOffset + 3,
//					indexOffset + 1
//				});
//
//			indexOffset += 4;
//		}
//	}
//
//	glBindBuffer(GL_ARRAY_BUFFER, vbo);
//	glBufferData(GL_ARRAY_BUFFER,
//		vertexBuffer.size() * sizeof(float),
//		vertexBuffer.data(),
//		GL_DYNAMIC_DRAW);
//
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
//		indexBuffer.size() * sizeof(unsigned int),
//		indexBuffer.data(),
//		GL_DYNAMIC_DRAW);
//}

TrailRenderer::TrailInstance TrailRenderer::BuildOrUpdateTrailInstance(
	const TrailRenderer::BasicTrailData& trail, const glm::vec3& cameraPos, TrailInstance* existingInstance)
{
	TrailInstance instance = existingInstance ? *existingInstance : TrailInstance();
	instance.vertexBuffer.clear();
	instance.indexBuffer.clear();

	if (trail.points.size() < 2)
		return instance;

	unsigned int indexOffset = 0;

	size_t segmentCount = trail.points.size() - 1;

	instance.vertexBuffer.reserve(segmentCount * 4 * 7);
	// 4 vertices per segment, 7 floats per vertex

	instance.indexBuffer.reserve(segmentCount * 6);
	// 6 indices per quad

	for (size_t i = 0; i < trail.points.size() - 1; ++i)
	{
		const auto& p0 = trail.points[i];
		const auto& p1 = trail.points[i + 1];

		float l0 = trail.lifetimes[i];
		float l1 = trail.lifetimes[i + 1];

		glm::vec3 dir = glm::normalize(p1 - p0);
		glm::vec3 toCamera = glm::normalize(cameraPos - p0);
		glm::vec3 side = glm::cross(dir, toCamera);
		if (glm::length2(side) < 1e-6f)
		{
			side = glm::cross(dir, glm::vec3(0, 1, 0));
			if (glm::length2(side) < 1e-6f)
				side = glm::cross(dir, glm::vec3(1, 0, 0));
		}
		side = glm::normalize(side);

		float t0 = glm::clamp(l0 / trail.maxLifetime, 0.0f, 1.0f);
		float t1 = glm::clamp(l1 / trail.maxLifetime, 0.0f, 1.0f);

		float w0 = trail.width * (1.0f - t0);
		float w1 = trail.width * (1.0f - t1);

		glm::vec3 p0L = p0 - side * w0 * 0.5f;
		glm::vec3 p0R = p0 + side * w0 * 0.5f;
		glm::vec3 p1L = p1 - side * w1 * 0.5f;
		glm::vec3 p1R = p1 + side * w1 * 0.5f;

		auto& vb = instance.vertexBuffer;

		vb.push_back(p0L.x); vb.push_back(p0L.y); vb.push_back(p0L.z);
		vb.push_back(trail.color.r); vb.push_back(trail.color.g); vb.push_back(trail.color.b); vb.push_back(1.0f - t0);

		vb.push_back(p0R.x); vb.push_back(p0R.y); vb.push_back(p0R.z);
		vb.push_back(trail.color.r); vb.push_back(trail.color.g); vb.push_back(trail.color.b); vb.push_back(1.0f - t0);

		vb.push_back(p1L.x); vb.push_back(p1L.y); vb.push_back(p1L.z);
		vb.push_back(trail.color.r); vb.push_back(trail.color.g); vb.push_back(trail.color.b); vb.push_back(1.0f - t1);

		vb.push_back(p1R.x); vb.push_back(p1R.y); vb.push_back(p1R.z);
		vb.push_back(trail.color.r); vb.push_back(trail.color.g); vb.push_back(trail.color.b); vb.push_back(1.0f - t1);

		instance.indexBuffer.push_back(indexOffset + 0);
		instance.indexBuffer.push_back(indexOffset + 2);
		instance.indexBuffer.push_back(indexOffset + 3);

		instance.indexBuffer.push_back(indexOffset + 0);
		instance.indexBuffer.push_back(indexOffset + 3);
		instance.indexBuffer.push_back(indexOffset + 1);

		indexOffset += 4;
	}



	instance.InitGL();
	instance.UpdateBuffers();

	return instance;
}

void TrailRenderer::RebuildTrailInstances(const glm::vec3& cameraPos)
{
	
	size_t i = 0;
	for (; i < trailData.size(); ++i)
	{
		if (i < trailsToDraw.size())
		{
			trailsToDraw[i] = BuildOrUpdateTrailInstance(trailData[i], cameraPos, &trailsToDraw[i]);
		}
		else
		{
			trailsToDraw.push_back(BuildOrUpdateTrailInstance(trailData[i], cameraPos));
		}
	}

	// Remove excess instances if any
	for (size_t j = i; j < trailsToDraw.size(); ++j)
		trailsToDraw[j].Cleanup();

	trailsToDraw.resize(trailData.size());
}

void TrailRenderer::Render(Shader& shader, const CameraData& camera)
{
	RebuildTrailInstances(camera.position);

	if (trailsToDraw.empty()) return;
	
	shader.Use();
	shader.SetTrans("projection", camera.GetPerspMtx());
	shader.SetTrans("view", camera.GetViewMtx());

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisable(GL_CULL_FACE);   // trails are thin quads
	glDisable(GL_DEPTH_TEST);  // temporarily disable for debugging

	for (auto& inst : trailsToDraw)
	{
		if (inst.indexBuffer.empty()) continue;

		glBindVertexArray(inst.vao);
		glDrawElements(GL_TRIANGLES, inst.indexBuffer.size(), GL_UNSIGNED_INT, 0);
	}

	shader.Disuse();
}