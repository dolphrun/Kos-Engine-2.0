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

    private:
        FMOD::System* s_fmod = nullptr;
        FMOD::Studio::System* m_studio = nullptr; 
        bool s_paused = false;
    };

} // namespace audio
