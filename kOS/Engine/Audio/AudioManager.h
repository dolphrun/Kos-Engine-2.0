#pragma once
#include <FMOD/fmod.hpp>
#include <FMOD/fmod_studio.hpp>


namespace FMOD {
    class System;
}

namespace audio {

    class AudioManager {
    public:

        AudioManager();                                 
        ~AudioManager();

        FMOD::System* GetCore() { return s_fmod; }
        FMOD::Studio::System* GetStudio() const { return m_studio; }

        void Init();
        void Update();

        void SetPaused(bool paused);
        void StopAll();

        void SetMasterVolume(float volume);   // 0.0f = silent, 1.0f = full
        void SetMusicVolume(float volume);    //For BGM bus separation
        void SetSFXVolume(float volume);      //For SFX bus separation

        float GetMasterVolume() const { return m_masterVolume; }

        FMOD::ChannelGroup* GetMusicGroup() const { return m_musicGroup; }
        FMOD::ChannelGroup* GetSFXGroup()   const { return m_sfxGroup; }

    private:
        FMOD::System* s_fmod = nullptr;
        FMOD::Studio::System* m_studio = nullptr; 
        bool s_paused = false;

        float m_masterVolume = 1.0f;
        FMOD::ChannelGroup* m_musicGroup = nullptr;
        FMOD::ChannelGroup* m_sfxGroup = nullptr;
        float m_musicVolume = 1.0f;
        float m_sfxVolume = 1.0f;

    };

} // namespace audio
