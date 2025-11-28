#include "Config/pch.h"
#include "R_AudioStudio.h"

void R_AudioStudio::Load()
{
    if (m_filePath.empty()) {
        return;
    }

    if (!std::filesystem::exists(m_filePath)) {
        std::cout << "[Audio] bank file not found: " << m_filePath.string() << "\n";
        return;
    }

    Unload();

    if (!m_studio) {
        std::cout << "[R_AudioStudio] Cannot load bank '" << m_filePath.string()
            << "': Studio system is null.\n";
        return;
    }

    FMOD_RESULT r = m_studio->loadBankFile(
        m_filePath.string().c_str(),
        FMOD_STUDIO_LOAD_BANK_NORMAL,
        &m_bank
    );

    if (r != FMOD_OK || !m_bank) {
        std::cout << "[R_AudioStudio] Failed to load bank: " << m_filePath.string() << "\n";
        m_bank = nullptr;
        return;
    }
}

void R_AudioStudio::Unload()
{
    if (m_bank) {
        m_bank->unload();
        m_bank = nullptr;
    }
}
