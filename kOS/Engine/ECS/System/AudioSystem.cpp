/******************************************************************/
/*!
\file      AudioSystem.cpp
\author    Chiu Jun Jie
\par       junjie.c@digipen.edu
\date      Oct 03, 2025
\brief     This file implements the definition of the AudioSystem
		   class. It handles the logic behind the Audio played in
		   the engine and works with the Audio Manager interface.


Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "AudioSystem.h"
#include "Resources/ResourceManager.h"
#include "Audio/AudioManager.h"
#include "AudioListenerSystem.h"

namespace ecs {

	void AudioSystem::Init() {
		m_audioManager.Init();

		auto* studio = m_audioManager.GetStudio();
		if (!studio) {
			//std::cout << "[AudioSystem] Studio system is null in Init()\n";

			return;
		}

		const auto& entities = m_entities.Data();

		for (EntityID id : entities) {
			auto* audioComp = m_ecs.GetComponent<AudioComponent>(id);
			if (!audioComp) continue;

			for (auto& af : audioComp->audioFiles) {

				if (af.sourceType != AudioSourceType::Studio)
					continue;

				if (af.audioBankGUID.Empty())
					continue;

				// Load bank 
				auto bankRes = m_resourceManager.GetResource<R_AudioStudio>(af.audioBankGUID);
				if (!bankRes) {
					//std::cout << "[AudioSystem] No R_AudioStudio resource for bank GUID\n";
					continue;
				}
				bankRes->SetStudio(studio);

				if (!bankRes->GetBank()) {
					bankRes->Load();
				}
			}
		}
	}

	inline FMOD_VECTOR ToF(const glm::vec3& v) { return FMOD_VECTOR{ v.x, v.y, v.z }; }

	void AudioSystem::Update() {

		//const auto& listenerEntities = m_ecs.GetComponentsEnties("AudioListenerComponent");

		//for (EntityID id : listenerEntities) {
		//	auto* listener = m_ecs.GetComponent<AudioListenerComponent>(id);
		//	auto* transform = m_ecs.GetComponent<TransformComponent>(id);

		//	if (!listener || !transform) continue;
		//	if (!listener->active)       continue;

		//	m_listenerPos = transform->WorldTransformation.position;

		//	break;
		//}

		//ecs::UpdateListenerFromComponents(m_ecs, m_listenerPos);

		if (auto* core = m_audioManager.GetCore()) {
			FMOD_VECTOR p = ToF(m_listenerPos);
			FMOD_VECTOR v{ 0,0,0 };
			FMOD_VECTOR f = ToF(m_listenerFwd);
			FMOD_VECTOR u = ToF(m_listenerUp);
			core->set3DListenerAttributes(0, &p, &v, &f, &u);
		}

		if (auto* studio = m_audioManager.GetStudio()) {
			FMOD_3D_ATTRIBUTES lis{};
			lis.position = ToF(m_listenerPos);
			lis.forward = ToF(m_listenerFwd);
			lis.up = ToF(m_listenerUp);
			lis.velocity = FMOD_VECTOR{ 0.0f, 0.0f, 0.0f };

			studio->setListenerAttributes(0, &lis);
		}

		const auto& entities = m_entities.Data();
		for (const EntityID id : entities) {
			auto* transform = m_ecs.GetComponent<TransformComponent>(id);
			auto* nameComp = m_ecs.GetComponent<NameComponent>(id);
			auto* audioComp = m_ecs.GetComponent<AudioComponent>(id);

			if (!nameComp || !audioComp) continue;

			if (nameComp->hide) continue;
			
			//Loop through all audio files
			for (auto& af : audioComp->audioFiles) {

				if (af.sourceType == AudioSourceType::Core && af.use3D && transform && af.channel) {
					FMOD::Channel* ch = static_cast<FMOD::Channel*>(af.channel);
					glm::vec3 pos = transform->WorldTransformation.position;

					/*std::cout << "[AudioSystem] 3D source '"
						<< (nameComp ? nameComp->entityName : "<no name>")
						<< "' pos = (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";*/

					FMOD_VECTOR fpos{ pos.x, pos.y, pos.z };
					FMOD_VECTOR fvel{ 0,0,0 };
					ch->set3DAttributes(&fpos, &fvel);
				}

				//update audio volume
				if (af.sourceType == AudioSourceType::Core && af.channel) {
					FMOD::Channel* ch = static_cast<FMOD::Channel*>(af.channel);

					bool isPlaying = false;
					if (ch->isPlaying(&isPlaying) == FMOD_OK && isPlaying) {

						if (std::abs(af.volume - af.lastVolume) > 0.001f) {
							ch->setVolume(std::clamp(af.volume, 0.0f, 1.0f));
							af.lastVolume = af.volume;
						}

						if (!af.use3D && std::abs(af.pan - af.lastPan) > 0.001f) {
							ch->setPan(std::clamp(af.pan, -1.0f, 1.0f));
							af.lastPan = af.volume; 
						}
					}
					else {
						af.channel = nullptr;
					}
				}


				if (af.sourceType == AudioSourceType::Studio && af.studioInstance) {
					auto* inst = static_cast<FMOD::Studio::EventInstance*>(af.studioInstance);

					FMOD_STUDIO_PLAYBACK_STATE state{};
					if (inst->getPlaybackState(&state) != FMOD_OK ||
						state == FMOD_STUDIO_PLAYBACK_STOPPED) {
						inst->release();

						af.studioInstance = nullptr;
					}
					else if (af.volume != af.lastVolume) {
						inst->setVolume(std::clamp(af.volume, 0.0f, 1.0f));
						af.lastVolume = af.volume;
					}

				}
				if (!af.requestPlay) continue;

				if (af.sourceType == AudioSourceType::Studio) {

					auto* studio = m_audioManager.GetStudio();
					if (!studio) {
						af.requestPlay = false; 
						continue;
					}

					if (af.studioEventPath.empty()) {
						af.requestPlay = false;
						continue;
					}

					FMOD::Studio::EventDescription* desc = nullptr;
					FMOD_RESULT ev = studio->getEvent(af.studioEventPath.c_str(), &desc);
					if (ev != FMOD_OK || !desc) {
						//std::cout << "[AudioSystem] getEvent FAILED for '"
						//	<< af.studioEventPath << "' result = " << ev << "\n";
						af.requestPlay = false;
						continue;
					}

					FMOD::Studio::EventInstance* inst = nullptr;
					if (desc->createInstance(&inst) != FMOD_OK || !inst) {
						af.requestPlay = false;
						continue;
					}

					// Volume
					inst->setVolume(std::clamp(af.volume, 0.0f, 1.0f));

					if (af.use3D && transform) {
						const glm::vec3 pos = transform->WorldTransformation.position;

						FMOD_3D_ATTRIBUTES attrs{};
						attrs.position = FMOD_VECTOR{ pos.x, pos.y, pos.z };
						attrs.forward = FMOD_VECTOR{ 0.0f, 0.0f, 1.0f };
						attrs.up = FMOD_VECTOR{ 0.0f, 1.0f, 0.0f };
						attrs.velocity = FMOD_VECTOR{ 0.0f, 0.0f, 0.0f };

						inst->set3DAttributes(&attrs);
					}

					inst->start();
					//inst->release();

					af.studioInstance = inst;
					af.lastVolume = af.volume;
					af.hasPlayed = true;
					af.requestPlay = false;
					continue;
				}

				if (af.audioGUID.Empty()) continue;

				//Get GUID
				auto res = m_resourceManager.GetResource<R_Audio>(af.audioGUID);
				if (!res) continue;

				//LOad sound
				if (!res->GetSound()) {
					res->SetSystem(m_audioManager.GetCore());

					res->Load();
				}

				FMOD::Sound* sound = res->GetSound();
				FMOD::System* system = res->GetSystem();

				if (!sound || !system) continue;

				//Player sound
				if (af.use3D) {
					sound->setMode(FMOD_3D | FMOD_3D_LINEARROLLOFF);

					const float minD = std::max(0.01f, af.minDistance);
					const float maxD = std::max(minD + 0.01f, af.maxDistance);
					sound->set3DMinMaxDistance(minD, maxD);
				}
				else {
					sound->setMode(FMOD_2D | FMOD_LOOP_OFF);
				}

				FMOD::Channel* ch = nullptr;
				if (system->playSound(sound, nullptr, true, &ch) == FMOD_OK && ch) {

					ch->setVolume(std::clamp(af.volume, 0.0f, 1.0f));

					if (af.use3D) {
						const glm::vec3 pos = transform ? transform->WorldTransformation.position : glm::vec3(0);
						const FMOD_VECTOR fpos{ pos.x, pos.y, pos.z };
						const FMOD_VECTOR fvel{ 0,0,0 };
						ch->set3DAttributes(&fpos, &fvel);
						ch->set3DLevel(1.0f);

						if (af.loop) {
							ch->setMode(FMOD_3D | FMOD_LOOP_NORMAL); 
							ch->setLoopCount(-1);
						}
						else {
							ch->setMode(FMOD_3D | FMOD_LOOP_OFF);
							ch->setLoopCount(0);
						}
					}
					else {
						ch->setPan(std::clamp(af.pan, -1.0f, 1.0f));
						if (af.loop) {
							ch->setMode(FMOD_2D | FMOD_LOOP_NORMAL); 
							ch->setLoopCount(-1);
						}
						else {
							ch->setMode(FMOD_2D | FMOD_LOOP_OFF);    
							ch->setLoopCount(0);
						}
					}

					ch->setPaused(false);
					af.channel = ch;
				}

				//Ensure only play once per frame
				af.requestPlay = false;
			}
		}

		//Update sound every frame
		m_audioManager.Update();

	}

	void AudioSystem::SetListener(const glm::vec3& pos,
		const glm::vec3& fwd,
		const glm::vec3& up) {
		m_listenerPos = pos;
		m_listenerFwd = fwd;
		m_listenerUp = up;
	}

	void AudioSystem::SetAudioPaused(EntityID id, const std::string& audioName, bool paused) {
		auto* audioComp = m_ecs.GetComponent<AudioComponent>(id);
		if (!audioComp) return;

		for (auto& af : audioComp->audioFiles) {
			if (af.name != audioName) continue;

			// --- Core ---
			if (af.sourceType == AudioSourceType::Core && af.channel) {
				FMOD::Channel* ch = static_cast<FMOD::Channel*>(af.channel);
				ch->setPaused(paused);
			}

			// --- Studio ---
			if (af.sourceType == AudioSourceType::Studio && af.studioInstance) {
				auto* inst = static_cast<FMOD::Studio::EventInstance*>(af.studioInstance);
				if (paused)
					inst->setPaused(true);
				else
					inst->setPaused(false);
			}
		}
	}

}
