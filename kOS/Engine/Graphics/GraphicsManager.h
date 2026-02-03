/********************************************************************/
/*!
\file      Graphics Manager.h
\author    Gabe Ng 2301290 Sean Tiu
\par       gabe.ng@digipen.edu
\date      Oct 03 2025
\brief     The main graphics manager for our system
		   It initializes all the necessary objects to be used for
		   rendering graphics.
		   Houses functions to be called on update as well

		   Rundown for the graphics system goes as follows
		   - Objects rendered to the Gbuffer
		   - Depth buffer is rendered
		   - Different rendering calls are called depending on the shader type

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#pragma once
#include "GraphicsReferences.h"
#include "Model.h"
#include "Mesh.h"
#include "Material.h"
#include "Framebuffer.h"
#include "Light.h"
#include "UIElements.h"
#include "Camera.h"
#include "Renderer.h"
#include "ShaderManager.h"
#include "FramebufferManager.h"
#include "Resources/ResourceManager.h"
#include "PostProcessing.h"
class GraphicsManager
{
public:
	//Singleton class, remove all copy and assignment operations
	GraphicsManager() = default;

	GraphicsManager(const GraphicsManager&) = delete;
	GraphicsManager& operator=(const GraphicsManager&) = delete;
	GraphicsManager(GraphicsManager&&) = delete;
	GraphicsManager& operator=(GraphicsManager&&) = delete;

	//Main Functions
	void gm_Initialize(float width, float height);
	void gm_Update();
	void gm_Render();
	void gm_RenderDebug();
	void gm_Clear();
	void gm_ClearGBuffer();
	void gm_RenderDeferredObjects(const CameraData& camera);
	void gm_ResetFrameBuffer();
	FramebufferManager* gm_GetFBM() { return &framebufferManager;; }

	//Data Transfer Functions
	inline void gm_PushScreenTextData(ScreenTextData&& fontData) { textRenderer.screenTextToDraw.emplace_back(std::move(fontData)); };
	inline void gm_PushMeshData(MeshData&& meshData) { meshRenderer.meshesToDraw[0].emplace_back(std::move(meshData)); };
	inline void gm_PushMeshData(MeshData&& meshData,layer::LAYERS index) { meshRenderer.meshesToDraw[index].emplace_back(std::move(meshData)); };
	inline void gm_PushScreenSpriteData(ScreenSpriteData&& spriteData) { spriteRenderer.screenSpritesToDraw.emplace_back(std::move(spriteData)); };
	inline void gm_PushWorldSpriteData(ScreenSpriteData&& spriteData) { spriteRenderer.worldSpriteToDraw.emplace_back(std::move(spriteData)); };
	inline void gm_PushPointLightData(PointLightData&& pointLightData) { lightRenderer.pointLightsToDraw.emplace_back(std::move(pointLightData)); };
	inline void gm_PushDirectionalLightData(DirectionalLightData&& directionalLightData) { lightRenderer.directionLightsToDraw.emplace_back(std::move(directionalLightData)); };
	inline void gm_PushSpotLightData(SpotLightData&& spotLightData) { lightRenderer.spotLightsToDraw.emplace_back(std::move(spotLightData)); };
	inline void gm_MoveEditorCameraData(const CameraData& camera) { editorCamera = camera; editorCameraActive = true; };
	inline void gm_PushGameCameraData(CameraData&& camera) { 
		//Push and sort
		gameCameras.emplace_back(std::move(camera)); 
		std::sort(gameCameras.begin(), gameCameras.end(), [](const CameraData& a, const CameraData& b) {return a.renderOrder > b.renderOrder; });
	};
	inline void gm_PushCubeDebugData(BasicDebugData&& data) { debugRenderer.basicDebugCubes.emplace_back(std::move(data)); };
	inline void gm_PushCapsuleDebugData(BasicDebugData&& data) { debugRenderer.basicDebugCapsules.emplace_back(std::move(data)); }
	inline void gm_PushSphereDebugData(BasicDebugData&& data) { debugRenderer.basicDebugSpheres.emplace_back(std::move(data)); }
	inline void gm_PushCubeData(CubeRenderer::CubeData&& data) { cubeRenderer.cubesToDraw.emplace_back(std::move(data)); };
	inline void gm_PushSphereData(SphereRenderer::SphereData&& data) { sphereRenderer.spheresToDraw.emplace_back(std::move(data)); };
	inline void gm_PushVideoData(VideoRenderer::VideoData&& videoData) { videoRenderer.vecVideoData.emplace_back(std::move(videoData)); };
	void gm_DrawMaterial(const PBRMaterial& md, FrameBuffer& fb);
	inline void gm_PushSkinnedMeshData(SkinnedMeshData&& skinnedMeshData) {
		skinnedMeshRenderer.skinnedMeshesToDraw[0].emplace_back(std::move(skinnedMeshData));
		skinnedMeshRenderer.skinnedMeshLookup[skinnedMeshRenderer.skinnedMeshesToDraw[0].back().entityID]
			= &skinnedMeshRenderer.skinnedMeshesToDraw[0].back();
	};
	inline void gm_PushSkinnedMeshData(SkinnedMeshData&& skinnedMeshData,layer::LAYERS layer) {
		skinnedMeshRenderer.skinnedMeshesToDraw[layer].emplace_back(std::move(skinnedMeshData));
		skinnedMeshRenderer.skinnedMeshLookup[skinnedMeshRenderer.skinnedMeshesToDraw[layer].back().entityID]
			= &skinnedMeshRenderer.skinnedMeshesToDraw[0].back();
	};
	inline void gm_PushBasicParticleData(BasicParticleData&& basicParticleData) { particleRenderer.particlesToDraw.emplace_back(std::move(basicParticleData)); };
	


	//Accessors
	inline const FrameBuffer& gm_GetEditorBuffer() const { return framebufferManager.editorBuffer; };
	inline const FrameBuffer& gm_GetGameBuffer() const { return framebufferManager.gameBuffer; };
	void gm_FillDepthCube(const CameraData&, int,glm::vec3 lighPos);
	void gm_FillDepthCube(const CameraData&, int, glm::vec3 lighPos, std::vector<MeshData>const&);

	void gm_UpdateBuffers(int width, int height);
	void gm_RenderGameBuffer();
	void gm_updatemouse(GLFWwindow* glwin) {

		double mx, my;
		glfwGetCursorPos(glwin, &mx, &my);

		int winW, winH;
		glfwGetWindowSize(glwin, &winW, &winH);

		isButtonPressed = glfwGetMouseButton(glwin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
		if (mx >= 0 && mx <= winW &&
			my >= 0 && my <= winH)
		{
			float relX = static_cast<float>(mx / winW);
			float relY = static_cast<float>(my / winH);

			int fbW, fbH;
			glfwGetFramebufferSize(glwin, &fbW, &fbH);

			int pixelX = int(relX * fbW);
			int pixelY = fbH - int(relY * fbH);

			GLuint fbo;
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D,
				gm_GetFBM()->gBuffer.gMaterial,
				0
			);

			float pixelVal;
			glReadPixels(
				pixelX,
				pixelY,
				1,
				1,
				GL_ALPHA,
				GL_FLOAT,
				&pixelVal
			);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &fbo);

			--pixelVal;
			buttonID = (pixelVal < 0.0f) ? -1 : static_cast<int>(pixelVal);
		}
	}
	//I want my DCMs
	LightRenderer lightRenderer;

	int buttonID{ -1 };
	bool isButtonPressed{ false };

	DebugNavMesh* renderNavMesh;
	//Reference post processing profile
	PostProcessingProfile* postProcessProfile;
private:

	//Initialize functions
	void gm_InitializeMeshes();

	//Render functions
	void gm_RenderToEditorFrameBuffer();
	void gm_RenderToGameFrameBuffer();
	void gm_FillDataBuffers(const CameraData& camera);
	void gm_FillDataBuffersGame(const CameraData& camera);
	void gm_FillDataBuffersGame(const CameraData& camera,layer::LAYERS);

	void gm_FillGBuffer(const CameraData& camera);

	void gm_FillDepthBuffer(const CameraData& camera);
	void gm_FillDepthCube(const CameraData& camera);
	void gm_RenderCubeMap(const CameraData& camera);
	void gm_RenderDebugObjects(const CameraData& camera);
	void gm_RenderParticles(const CameraData& camera);
	void gm_RenderUIObjects(const CameraData& camera);
	void gm_RenderVideo(const CameraData& camera);

	void gm_FillGBufferGame(const CameraData& camera);
	void gm_FillGBufferGame(const CameraData& camera, layer::LAYERS);

	unsigned int* gm_PostProcess();
	//Cameras
	CameraData editorCamera{};
	std::vector<CameraData> gameCameras{};
	bool editorCameraActive{ false };
	int currentGameCameraIndex{};

	//Renderers
	TextRenderer textRenderer;
	SpriteRenderer spriteRenderer;
	MeshRenderer meshRenderer;
	SkinnedMeshRenderer skinnedMeshRenderer;
	DebugRenderer debugRenderer;
	CubeRenderer cubeRenderer;
	SphereRenderer sphereRenderer;
	ParticleRenderer particleRenderer;
	VideoRenderer videoRenderer;
	//Managers
	ShaderManager shaderManager;
	FramebufferManager framebufferManager;

	Cube cube;
	Sphere sphere;
	//Viewport sizes
	float windowWidth, windowHeight;

};