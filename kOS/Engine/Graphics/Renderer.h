/********************************************************************/
/*!
\file      Renderer.h
\author    Sean Tiu (2303398)
\par       Emails: s.tiu@digipen.edu
\date      Oct 03, 2025
\brief     Declares various renderer structures used in the graphics
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
#pragma once
#include"GraphicsReferences.h"
#include "Resources/ResourceHeader.h"
#include "Mesh.h"
#include "Light.h"
#include "UIElements.h"
#include "Model.h"
#include "Camera.h"
#include "Particle.h"
#include <vector>
#include "CubeMap.h"

struct BasicRenderer
{
	virtual void Clear() = 0;
};

struct MeshRenderer : BasicRenderer
{
	void Render(const CameraData& camera, Shader& shader);
	void Render(const CameraData& camera, Shader& shader,layer::LAYERS);
	void Clear() override;
	std::array<std::vector<MeshData>, layer::MAXLAYER> meshesToDraw{};
	//std::array
};

struct SkinnedMeshRenderer : BasicRenderer
{
	void Update();
	void Render(const CameraData& camera, Shader& shader);
	void Render(const CameraData& camera, Shader& shader, layer::LAYERS);
	void Clear() override;
	std::array < std::vector<SkinnedMeshData>, layer::MAXLAYER> skinnedMeshesToDraw{};
	std::unordered_map<unsigned int, SkinnedMeshData*> skinnedMeshLookup{}; //Entity ID, Skinned Mesh Pointer
};

struct CubeRenderer : BasicRenderer
{
	struct CubeData {
		PBRMaterial meshMaterial;
		glm::mat4 transformation{ 1.f };
		int entityID{ -1 };
	};
	void Render(const CameraData& camera, Shader& shader, Cube* cubePtr);
	void Clear();
	std::vector<CubeData> cubesToDraw{};
};
struct SphereRenderer:BasicRenderer
{
	struct SphereData {
		PBRMaterial meshMaterial;
		glm::mat4 transformation{ 1.f };
		int entityID{ -1 };
	};
	void Render(const CameraData& camera, Shader& shader, Sphere* cubePtr);
	void Clear();
	std::vector<SphereData> spheresToDraw{};

};
struct TextRenderer : BasicRenderer
{
	void InitializeTextRendererMeshes();
	void RenderScreenFonts(const CameraData& camera, Shader& shader);
	void Clear() override;
	std::vector<ScreenTextData> screenTextToDraw{};
	///std::vector<WorldTextData> worldTextToDraw{};
	TextMesh screenTextMesh;
};

struct SpriteRenderer : BasicRenderer
{
	void InitializeSpriteRendererMeshes();
	void RenderScreenSprites(const CameraData& camera, Shader& shader);
	void RenderWorldSprites(const CameraData& camera, Shader& shader);
	void Clear() override;
	std::vector<ScreenSpriteData> screenSpritesToDraw{};
	std::vector<ScreenSpriteData> worldSpriteToDraw{};

private:
	ScreenSpriteMesh screenSpriteMesh;
};

struct LightRenderer : BasicRenderer
{
	void InitializeLightRenderer();
	void UpdateDCM();
	void RenderAllLights(const CameraData& camera, Shader& shader);
	void DebugRender(const CameraData& camera, Shader& shader);
	void Clear() override;
	std::vector<PointLightData> pointLightsToDraw{};
	std::vector<DirectionalLightData> directionLightsToDraw{};
	std::vector<SpotLightData> spotLightsToDraw{};
	DepthCubeMap dcm[32];
	std::vector<DepthCubeMap*> actualDcm;
	std::vector<PointLightData*> actualPointLightsToDraw{};

	DepthCubeMap testDCM;
};

struct DebugRenderer : BasicRenderer {
	void InitializeDebugRendererMeshes();
	void Render(const CameraData& camera, Shader& shader);
	void RenderPointLightDebug(const CameraData& camera, Shader& shader, std::vector<PointLightData> pointLights);
	void RenderDebugFrustums(const CameraData& camera, Shader& shader, const std::vector<CameraData>& debugFrustums);
	void RenderDebugCubes(const CameraData& camera, Shader& shader);
	void RenderDebugCapsules(const CameraData& camera, Shader& shader);
	void RenderDebugSpheres(const CameraData& camera, Shader& shader);
	void RenderDebugMeshes(const CameraData& camera, Shader& shader);
	void RenderDebugLines(const CameraData& camera, Shader& shader);
	void Clear() override;

	std::vector<BasicDebugData> basicDebugCubes{};
	std::vector<BasicDebugData> basicDebugCapsules{};
	std::vector<BasicDebugData> basicDebugSpheres{};
	std::vector<DebugMeshData> basicDebugMeshes{};
	std::vector<DebugLineData> basicDebugLines{};
private:
	DebugCircle debugCircle;
	DebugFrustum debugFrustum;
	DebugCube debugCube;
	DebugCapsule debugCapsule;
	DebugMesh debugMesh;
	Line debugLine;
};

struct TrailRenderer : BasicRenderer {
public:
	struct TrailInstance
	{
		std::vector<float> vertexBuffer;
		std::vector<unsigned int> indexBuffer;

		unsigned int vao = 0;
		unsigned int vbo = 0;
		unsigned int ebo = 0;

		bool initialized = false;

		void InitGL()
		{
			if (initialized) return; // only once
			glGenVertexArrays(1, &vao);
			glGenBuffers(1, &vbo);
			glGenBuffers(1, &ebo);

			glBindVertexArray(vao);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

			// Vertex layout: pos(3) + color(4)
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
			glEnableVertexAttribArray(1);

			glBindVertexArray(0);
			initialized = true;
		}

		void UpdateBuffers()
		{
			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float),
				vertexBuffer.data(), GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBuffer.size() * sizeof(unsigned int),
				indexBuffer.data(), GL_DYNAMIC_DRAW);
		}

		void Cleanup()
		{
			if (!initialized) return;
			glDeleteVertexArrays(1, &vao);
			glDeleteBuffers(1, &vbo);
			glDeleteBuffers(1, &ebo);
			vao = vbo = ebo = 0;
			initialized = false;
			vertexBuffer.clear();
			indexBuffer.clear();
		}
	};

	struct BasicTrailData
	{
		std::vector<glm::vec3> points;
		std::vector<float> lifetimes;

		float maxLifetime = 5.0f;
		float width = 10.f;
		glm::vec4 color{ 1.0f };
		glm::vec3 lastPosition{};
	};

	unsigned int vao{};
	unsigned int vbo{};
	unsigned int ebo{};

	std::vector<BasicTrailData> trailData{};
	std::vector<TrailInstance> trailsToDraw{};

	void InitTrailRendererMeshes();
	//void BuildTrailMesh(const std::vector<BasicTrailData>& trails, const glm::vec3& cameraPos);
	TrailInstance BuildOrUpdateTrailInstance(const TrailRenderer::BasicTrailData& trail, const glm::vec3& cameraPos, TrailInstance* existingInstance = nullptr);
	void RebuildTrailInstances(const glm::vec3& cameraPos);
	void Render(Shader& shader, const CameraData& camera);

	void Clear() override
	{
		trailData.clear();
		trailsToDraw.clear();
	};
};

struct ParticleRenderer : BasicRenderer {
	void InitializeParticleRendererMeshes();
	void Render(const CameraData& camera, Shader& shader, TrailRenderer& trailRenderer);
	void Clear() override;
	std::vector<BasicParticleData> particlesToDraw{};
	std::vector<BasicParticleInstance> instancedBasicParticles{};

private:
	BasicParticleMesh basicParticleMesh;
};



struct VideoRenderer : BasicRenderer {
	struct VideoData
	{
		glm::mat4 transformation;
		std::shared_ptr<R_Video> video;
		bool pause;
	};
	void InitializeVideoRendererMeshes();
	void Update(Shader& shader);
	void Render(const CameraData& camera, Shader& shader);
	void Clear() override;
	std::vector<VideoData> vecVideoData;
private:
	VideoMesh m_videoMesh;
};

