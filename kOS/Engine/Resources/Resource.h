/******************************************************************/
/*!
\file      Resource.h
\author    Jaz Winn Ng
\par       jazwinn.ng@digipen.edu
\date      Sept 28, 2025
\brief     Resource interface for saving and loading components in ECS.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/
#pragma once

#include "Config/pch.h"

class Resource{

public:
	Resource() = delete;
	Resource(utility::GUID GUID, std::filesystem::path filepath):m_GUID(GUID), m_filePath(filepath) {}
	Resource(std::filesystem::path filepath): m_filePath(filepath) {}

	virtual ~Resource() noexcept = default;

	virtual void Load() = 0;
	virtual void Unload() = 0;

	const inline utility::GUID GetGUID() const { return m_GUID; }
	const inline std::filesystem::path GetFilePath() const { return m_filePath; }


//protected:

	utility::GUID m_GUID;
	std::filesystem::path m_filePath;

};
