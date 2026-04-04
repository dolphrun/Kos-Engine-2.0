#include "AudioManager.h"

namespace audio {
    AudioManager::AudioManager() { s_fmod = nullptr; s_paused = false; }

    AudioManager::~AudioManager(){}

    void AudioManager::Init() {

        if (s_fmod || m_studio) return; 

        //create studio sys
        FMOD::Studio::System* studio = nullptr;
        FMOD_RESULT r = FMOD::Studio::System::create(&studio);
        if (r != FMOD_OK || !studio) {
            return;
        }

        //Initialise studio and core
        r = studio->initialize(
            512,
            FMOD_STUDIO_INIT_NORMAL,
            FMOD_INIT_NORMAL,
            nullptr
        );

        if (r != FMOD_OK) {
            studio->release();
            return;
        }


        FMOD::System* core = nullptr;
        r = studio->getCoreSystem(&core);
       // FMOD_RESULT fr = FMOD::System_Create(&core);
        if (r != FMOD_OK || !core)
        {
            studio->release();
            return;
        }

        core->set3DSettings(1.0f, 1.0f, 1.0f);

        // initialize core
        //fr = core->init(512, FMOD_INIT_NORMAL, nullptr);
        //if (fr != FMOD_OK) { core->release(); return; }

        s_fmod = core;
        s_paused = false;
        m_studio = studio;

        // Create named groups for independent volume control
        s_fmod->createChannelGroup("Music", &m_musicGroup);
        s_fmod->createChannelGroup("SFX", &m_sfxGroup);
    }

    void AudioManager::Update() {
        if (m_studio) {
            m_studio->update();
        }
        else if (s_fmod) {
            s_fmod->update();
        }
    }

    void AudioManager::SetPaused(bool paused) {
        s_paused = paused;

        if (m_studio) { 
            FMOD::Studio::Bus* masterBus = nullptr;
            if (m_studio->getBus("bus:/", &masterBus) == FMOD_OK && masterBus) {
                masterBus->setPaused(paused);
            }
        }

        if (!s_fmod) return;

        FMOD::ChannelGroup* master = nullptr;
        if (s_fmod->getMasterChannelGroup(&master) == FMOD_OK && master) {
            master->setPaused(paused);
        }
    }

    void AudioManager::StopAll() {
        if (!s_fmod) return;

        if (m_studio) {
            FMOD::Studio::Bus* masterBus = nullptr;
            if (m_studio->getBus("bus:/", &masterBus) == FMOD_OK && masterBus) {
                masterBus->stopAllEvents(FMOD_STUDIO_STOP_IMMEDIATE);
            }
        }

        FMOD::ChannelGroup* master = nullptr;
        if (s_fmod->getMasterChannelGroup(&master) == FMOD_OK && master) {
            master->stop();
        }
    }

    void AudioManager::SetMasterVolume(float volume) {
        // Clamp between 0 and 1
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;

        m_masterVolume = volume;

        // Set on FMOD Studio master bus (covers all studio events)
        if (m_studio) {
            FMOD::Studio::Bus* masterBus = nullptr;
            if (m_studio->getBus("bus:/", &masterBus) == FMOD_OK && masterBus) {
                masterBus->setVolume(volume);
            }
        }

        // Set on core master channel group (covers all raw createSound calls)
        if (s_fmod) {
            FMOD::ChannelGroup* master = nullptr;
            if (s_fmod->getMasterChannelGroup(&master) == FMOD_OK && master) {
                master->setVolume(volume);
            }
        }
    }

    void AudioManager::SetMusicVolume(float volume) {
        if (volume < 0.f) volume = 0.f;
        if (volume > 1.f) volume = 1.f;
        m_musicVolume = volume;
        if (m_musicGroup) m_musicGroup->setVolume(volume);
    }

    void AudioManager::SetSFXVolume(float volume) {
        if (volume < 0.f) volume = 0.f;
        if (volume > 1.f) volume = 1.f;
        m_sfxVolume = volume;
        if (m_sfxGroup) m_sfxGroup->setVolume(volume);
    }
} 
