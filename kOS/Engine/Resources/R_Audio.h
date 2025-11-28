/******************************************************************/
/*!
\file      R_Audio.h
\author    Chiu Jun Jie
\par
\date      Oct 03, 2025
\brief     This file contains the definition of the Audio Resource
           class. It handles the loading and unloading of the Audio
           through the resource manager.


Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#pragma once
#include "Config/pch.h"
#include "Resource.h"
#include <FMOD/fmod.hpp>
#include <FMOD/fmod_studio.hpp>

namespace FMOD { class System; class Sound; }

//enum class AudioAssetType {
//    CoreSound,  
//    StudioBank 
//};

class R_Audio : public Resource {
public:
    using Resource::Resource;

    void Load() override;
    void Unload() override;

    ~R_Audio() override { Unload(); }

    FMOD::Sound* GetSound()  const { return m_sound; }
    FMOD::System* GetSystem() const { return m_system; }
    void SetSystem(FMOD::System* sys) { m_system = sys; }

    //FMOD::Studio::Bank* GetBank()   const { return m_bank; }
    //FMOD::Studio::System* GetStudio() const { return m_studio; }
    //void SetStudio(FMOD::Studio::System* studio) { m_studio = studio; }

    //AudioAssetType GetType() const { return m_type; }


    REFLECTABLE(R_Audio);

private:
    //AudioAssetType m_type = AudioAssetType::CoreSound;

    FMOD::System* m_system = nullptr;        
    FMOD::Sound* m_sound = nullptr;       

    //FMOD::Studio::System* m_studio = nullptr;
    //FMOD::Studio::Bank* m_bank = nullptr;

    unsigned int  m_createFlags = 0;
};
