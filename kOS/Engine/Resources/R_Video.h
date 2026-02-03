/******************************************************************/
/*!
\file      Video.h
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 01, 2025
\brief     This file contains the declaration for the video object.


Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#ifndef VIDEO_H
#define VIDEO_H
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "ECS/ECSList.h"
#include "Config/pch.h"
#include "Resource.h"

#include <PL_MPEG/pl_mpeg.h>

enum VIDEO_FLAGS {
	AUDIO,
	LOOP,
	TOTAL
};

class R_Video : public Resource
{
public:

	using Resource::Resource;

	void Load() override;
	void Init(std::bitset<VIDEO_FLAGS::TOTAL> flag);

	~R_Video() {
		Unload();
	}

	void Unload() override;

	void DecodeAndUpdateVideo(GLuint shader, bool pause = false);

	bool HasStopped();

	GLuint yTexture, uTexture, vTexture;

	int videoframes;

	REFLECTABLE(R_Video);

private:
	void UpdateTextures(GLuint shader, plm_frame_t* frame);

	plm_t* mpeg;

	int videoWidth, videoHieght;

	std::map<std::string, int> uniformLocations;

	GLuint uni_yTexture, uni_uTexture, uni_vTexture;

	float elapsedTime;
	int videoFrameIndex = 0;
	std::chrono::high_resolution_clock::time_point lastTime;
};


//class VideoManager {






//public:

//	std::unordered_map<std::string, std::filesystem::path> m_videopath; //stores path of video

//	std::unordered_map<ecs::EntityID, std::unique_ptr<Video>> m_videoMap; // store all playing videos

//};



#endif VIDEO_H



