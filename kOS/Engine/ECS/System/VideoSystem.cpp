/******************************************************************/
/*!
\file      VideoSystem.cpp
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Sept 29, 2024
\brief   



Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "ECS/ECS.h"
#include "ECS/ecs.h"
#include "VideoSystem.h"
#include "Utility/MathUtility.h"

namespace ecs {
	
	void VideoSystem::Init(){
		//onRegister.Add([&](EntityID id) {
		//	VideoComponent* videoCom = m_ecs.GetComponent<VideoComponent>(id);
		//	m_videoMap[id] = m_resourceManager.GetDetachedResource<R_Video>(videoCom->videoGUID);
		//	std::bitset<VIDEO_FLAGS::TOTAL> videoFlags;
		//	videoFlags.set(VIDEO_FLAGS::AUDIO);
		//	if (videoCom->loop) {
		//		videoFlags.set(VIDEO_FLAGS::LOOP);
		//	}

		//	m_videoMap[id]->Init(videoFlags);


		//	});

		onDeregister.Add([&](EntityID id) {

			if (m_videoMap.find(id) != m_videoMap.end()) {
				m_videoMap[id]->Unload();
				m_videoMap.erase(id);
			}


			});



	}

	void VideoSystem::Update() {
		const auto& entities = m_entities.Data();
		for (const EntityID id : entities) {
			
			TransformComponent* transformCom = m_ecs.GetComponent<TransformComponent>(id);
			VideoComponent* videoCom = m_ecs.GetComponent<VideoComponent>(id);

			if (videoCom->playing == false) {
				m_videoMap[id]->Unload();
				m_videoMap.erase(id);

				continue;
			}
			
			if (m_videoMap.find(id) == m_videoMap.end()) {

				if (videoCom->videoGUID.Empty())continue;

				m_videoMap[id] = m_resourceManager.GetDetachedResource<R_Video>(videoCom->videoGUID);
				std::bitset<VIDEO_FLAGS::TOTAL> videoFlags;
				videoFlags.set(VIDEO_FLAGS::AUDIO);
				if (videoCom->loop) {
					videoFlags.set(VIDEO_FLAGS::LOOP);
				}

				m_videoMap[id]->Init(videoFlags);
			}


			//push to graphics manager
			m_graphicsManager.gm_PushVideoData(VideoRenderer::VideoData(glm::mat4(transformCom->transformation), m_videoMap.at(id), videoCom->pause));



			if (m_videoMap[id]->HasStopped()) {
				videoCom->playing = false;
			}
		}
	}
}
