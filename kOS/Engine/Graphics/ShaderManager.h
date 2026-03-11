/********************************************************************/
/*!
\file      ShaderManager.h
\author    Sean Tiu (2303398)
\par       Emails: s.tiu@digipen.edu
\date      Oct 03, 2025
\brief     Declares the ShaderManager class, which manages and
		   initializes all core engine shaders used throughout
		   the rendering system.

		   The ShaderManager is responsible for:
		   - Initializing and storing precompiled engine shaders.
		   - Managing a central collection of Shader objects
			 accessible via string keys.
		   - Providing shader programs for different rendering
			 stages, including:
			   * Default rendering
			   * Deferred PBR lighting
			   * G-buffer generation
			   * Depth mapping
			   * Skybox rendering
			   * Screen-space text and sprite rendering
			   * Framebuffer and composition passes

		   Shader source code is embedded directly into the binary
		   using preprocessor includes for portability and
		   efficient loading during engine startup.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#pragma once
#include "Shader.h"

class ShaderManager
{
public:
	void Initialize()
	{
		engineShaders.insert({ "DefaultDraw",Shader(defaultDrawVS, defaultDrawFS) });
		engineShaders.insert({ "DeferredPBRShader",Shader(deferredPBRVS, deferredPBRFS) });
		engineShaders.insert({ "GBufferPBRShader",Shader(gBufferPBRVS, gBufferPBRFS) });
		engineShaders.insert({ "GBufferDebugShader",Shader(gBufferDebugVS, gBufferDebugFS) });

		engineShaders.insert({ "DepthMapShader",Shader(depthMapVS, depthMapFS) });
		engineShaders.insert({ "SkyBoxShader",Shader(skyBoxVS, skyBoxFS) });
		engineShaders.insert({ "ScreenFontShader",Shader(screenFontVS, screenFontFS) });
		engineShaders.insert({ "ScreenSpriteShader",Shader(screenSpriteVS, screenSpriteFS) });
		engineShaders.insert({ "FrameBufferShader",Shader(frameBufferVS, frameBufferFS) });
		engineShaders.insert({ "FBOCompositeShader",Shader(fboCompositeVS, fboCompositeFS) });
		engineShaders.insert({ "MaterialShader",Shader(materialVS, materialFS) });
		engineShaders.insert({ "PointShadowShader",Shader(pointShadowVS, pointShadowFS,pointShadowGS) });
		//engineShaders.insert({ "BasicParticleShader", Shader(basicParticleVS,basicParticleFS) });
		//Experiment with creating a particle shader
		engineShaders.insert({ "GBufferParticleShader", Shader(basicParticleVS,basicParticleFS) });
		engineShaders.insert({ "FowardParticleShader",Shader(fwdParticleVS,fwdParticleFS) });
		engineShaders.insert({ "GBufferWorldShader", Shader(worldSpriteVS,worldSpriteFS) });
		engineShaders.insert({ "VignietteShader", Shader(vignietteShaderVS,vignietteShaderFS)});
		engineShaders.insert({ "FilmGrainShader",Shader(filmGrainVS,filmGrainFS) });
		engineShaders.insert({ "ChromaticAbberrationShader",Shader(chromAbbVS,chromAbbFS) });
		engineShaders.insert({ "VideoShader",Shader(videoVS,videoFS) });
		engineShaders.insert({ "TrailShader", Shader(trailVS, trailFS) });
		engineShaders.insert({ "BlurShader",Shader(blurVS,blurFS) });
		engineShaders.insert({ "DownSamplingShader",Shader(downSamplingVS,downSamplingFS) });
		engineShaders.insert({ "UpSamplingShader",Shader(upSamplingVS,upSamplingFS) });

	}

	std::unordered_map<std::string, Shader> engineShaders;
private:
	

	/*--------------------------------------------------------------------------*/
	/*---------------------------CORE ENGINE SHADERS----------------------------*/
	/*--------------------------------------------------------------------------*/
	const char* defaultDrawVS
	{
		#include "CoreEngineShaders/Shaders/DefaultDraw/DefaultDraw.vs"
	};
	const char* defaultDrawFS
	{
		#include "CoreEngineShaders/Shaders/DefaultDraw/DefaultDraw.fs"
	};
	const char* materialVS
	{
		#include "CoreEngineShaders/Shaders/MaterialShader/MaterialShader.vs"
	};
	const char* materialFS
	{
		#include "CoreEngineShaders/Shaders/MaterialShader/MaterialShader.fs"
	};

	const char* pointShadowVS
	{
		#include "CoreEngineShaders/Shaders/PointShadowShader/PointShadowShader.vs"
	};
	const char* pointShadowFS
	{
		#include "CoreEngineShaders/Shaders/PointShadowShader/PointShadowShader.fs"
	};
	const char* pointShadowGS
	{
		#include "CoreEngineShaders/Shaders/PointShadowShader/PointShadowShader.gs"
	};

	const char* frameBufferVS
	{
		#include "CoreEngineShaders/Shaders/FrameBuffShader/FrameBuffShader.vs"
	};
	const char* frameBufferFS
	{
		#include "CoreEngineShaders/Shaders/FrameBuffShader/FrameBuffShader.fs"
	};
	const char* fboCompositeVS
	{
		#include "CoreEngineShaders/Shaders/FBOCompositeShader/FBOCompositeShader.vs"
	};
	const char* fboCompositeFS
	{
		#include "CoreEngineShaders/Shaders/FBOCompositeShader/FBOCompositeShader.fs"
	};
	const char* deferredPBRVS
	{
		#include "CoreEngineShaders/Shaders/DeferredPBR/DeferredPBR.vs"
	};
	const char* deferredPBRFS
	{
		#include "CoreEngineShaders/Shaders/DeferredPBR/DeferredPBR.fs"
	};
	const char* gBufferPBRVS
	{
		#include "CoreEngineShaders/Shaders/GBuffPBRShader/GBuffPBRShader.vs"
	};
	const char* gBufferPBRFS
	{
		#include "CoreEngineShaders/Shaders/GBuffPBRShader/GBuffPBRShader.fs"
	};
	const char* gBufferDebugVS
	{
		#include "CoreEngineShaders/Shaders/GBufferDebugShader/GBufferDebugShader.vs"
	};
	const char* gBufferDebugFS
	{
		#include "CoreEngineShaders/Shaders/GBufferDebugShader/GBufferDebugShader.fs"
	};
	const char* depthMapVS
	{
		#include "CoreEngineShaders/Shaders/DepthMap/DepthMap.vs"
	};
	const char* depthMapFS
	{
		#include "CoreEngineShaders/Shaders/DepthMap/DepthMap.fs"
	};
	const char* skyBoxVS
	{
		#include "CoreEngineShaders/Shaders/SkyBoxShader/SkyBoxShader.vs"
	};
	const char* skyBoxFS
	{
		#include "CoreEngineShaders/Shaders/SkyBoxShader/SkyBoxShader.fs"
	};
	const char* screenFontVS
	{
		#include "CoreEngineShaders/Shaders/ScreenFontShader/ScreenFontShader.vs"
	};
	const char* screenFontFS
	{
		#include "CoreEngineShaders/Shaders/ScreenFontShader/ScreenFontShader.fs"
	};
	const char* screenSpriteVS
	{
		#include "CoreEngineShaders/Shaders/ScreenSpriteShader/ScreenSpriteShader.vs"
	};
	const char* screenSpriteFS
	{
		#include "CoreEngineShaders/Shaders/ScreenSpriteShader/ScreenSpriteShader.fs"
	};
	const char* worldSpriteVS
	{
		#include "CoreEngineShaders/Shaders/WorldSpriteShader/WorldSpriteShader.vs"
	};
	const char* worldSpriteFS
	{
		#include "CoreEngineShaders/Shaders/WorldSpriteShader/WorldSpriteShader.fs"
	};
	const char* basicParticleVS
	{
		#include "CoreEngineShaders/Shaders/BasicParticleShader/BasicParticleShader.vs"
	};
	const char* basicParticleFS
	{
		#include "CoreEngineShaders/Shaders/BasicParticleShader/BasicParticleShader.fs"
	};
	const char* fwdParticleVS
	{
		#include "CoreEngineShaders/Shaders/FowardParticleShader/FowardParticleShader.vs"
	};
	const char* fwdParticleFS
	{
		#include "CoreEngineShaders/Shaders/FowardParticleShader/FowardParticleShader.fs"
	};

	const char* vignietteShaderVS
	{
		#include "CoreEngineShaders/Shaders/VignietteShader/VignietteShader.vs"
	};
	const char* vignietteShaderFS
	{
		#include "CoreEngineShaders/Shaders/VignietteShader/VignietteShader.fs"
	};
	const char* filmGrainVS{
		#include "CoreEngineShaders/Shaders/FilmGrainShader/FilmGrain.vs"
	};
	const char* filmGrainFS{
		#include "CoreEngineShaders/Shaders/FilmGrainShader/FilmGrain.fs"
	};

	const char* chromAbbVS{
		#include "CoreEngineShaders/Shaders/ChromaticAbberationShader/ChromaticAbberation.vs"
	};
	const char* chromAbbFS{
		#include "CoreEngineShaders/Shaders/ChromaticAbberationShader/ChromaticAbberation.fs"
	};

	const char* videoVS{
		#include "CoreEngineShaders/Shaders/VideoShader/videoVertexShader.vs"
	};
	const char* videoFS{
		#include "CoreEngineShaders/Shaders/VideoShader/videoFragmentShader.fs"
	};

	const char* trailVS{
		#include "CoreEngineShaders/Shaders/TrailShader/TrailShader.vs"
	};
	const char* trailFS{
		#include "CoreEngineShaders/Shaders/TrailShader/TrailShader.fs"
	};
	const char* blurVS{
	#include "CoreEngineShaders/Shaders/BlurShader/BlurShader.vs"
	};
	const char* blurFS{
		#include "CoreEngineShaders/Shaders/BlurShader/BlurShader.fs"
	};

	//sampling for post processing
	const char* downSamplingVS{
		#include "CoreEngineShaders/Shaders/DownSamplingShader/DownSamplingShader.vs"
	};
	const char* downSamplingFS{
		#include "CoreEngineShaders/Shaders/DownSamplingShader/DownSamplingShader.fs"
	};
	const char* upSamplingVS{
		#include "CoreEngineShaders/Shaders/UpSamplingShader/UpSamplingShader.vs"
	};
	const char* upSamplingFS{
		#include "CoreEngineShaders/Shaders/UpSamplingShader/UpSamplingShader.fs"
	};

};