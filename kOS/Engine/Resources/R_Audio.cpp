/******************************************************************/
/*!
\file      R_Audio.h
\author    Chiu Jun Jie
\par	   junjie.c@digipen.edu
\date      Oct 03, 2025
\brief     This file contains the definition of the Audio Resource
		   class. It handles the loading and unloading of the Audio
		   through the resource manager.


Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include "Config/pch.h"
#include "R_Audio.h"

void R_Audio::Load()
{
	if (m_filePath.empty()) {
		return;
	}

	//Check if file path exist
	if (!std::filesystem::exists(m_filePath)) {
		std::cout << "[Audio] file not found: " << m_filePath.string() << "\n";
		return;
	}

	//Reset sound if have sound
	Unload();

	////STUDIOBANKS
	//if (m_type == AudioAssetType::StudioBank) {
	//	if (!m_studio) {
	//		std::cout << "[R_Audio] Cannot load bank '" << m_filePath.string()
	//			<< "': Studio system is null.\n";
	//		return;
	//	}

	//	FMOD_RESULT r = m_studio->loadBankFile(
	//		m_filePath.string().c_str(),
	//		FMOD_STUDIO_LOAD_BANK_NORMAL,
	//		&m_bank
	//	);

	//	if (r != FMOD_OK || !m_bank) {

	//		m_bank = nullptr;
	//		return;
	//	}

	//	// Bank loaded successfully
	//	return;
	//}


	FMOD::System* sys = m_system;
	if (!sys) {
		std::cout << "[R_Audio] Cannot load sound '" << m_filePath.string()
			<< "': core system is null.\n";
		return;
	}

	unsigned int flags = (m_createFlags != 0) ? m_createFlags : FMOD_DEFAULT;


	FMOD_RESULT r = sys->createSound(
		m_filePath.string().c_str(), 
		flags, 
		nullptr, 
		&m_sound
	);

	if (r != FMOD_OK || !m_sound) {
		r = sys->createSound(m_filePath.string().c_str(),
			flags | FMOD_CREATESTREAM,
			nullptr,
			&m_sound);
		if (r != FMOD_OK || !m_sound) {
			m_sound = nullptr;
			return;
		}
	}
}

void R_Audio::Unload()
{
	if (m_sound) {
		FMOD_RESULT r = m_sound->release();
		m_sound = nullptr;
	}

	//if (m_bank) {
	//	m_bank->unload();
	//	m_bank = nullptr;
	//}
}
