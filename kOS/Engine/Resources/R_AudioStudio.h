#pragma once
#include "Config/pch.h"
#include "Resource.h"
#include <FMOD/fmod_studio.hpp>

namespace FMOD { namespace Studio { class System; class Bank; } }

class R_AudioStudio : public Resource {
public:
    using Resource::Resource;

    void Load() override;
    void Unload() override;

    ~R_AudioStudio() override { Unload(); }

    FMOD::Studio::Bank* GetBank()   const { return m_bank; }
    FMOD::Studio::System* GetStudio() const { return m_studio; }
    void SetStudio(FMOD::Studio::System* studio) { m_studio = studio; }

    REFLECTABLE(R_AudioStudio);

private:
    FMOD::Studio::System* m_studio = nullptr;
    FMOD::Studio::Bank* m_bank = nullptr;
};
