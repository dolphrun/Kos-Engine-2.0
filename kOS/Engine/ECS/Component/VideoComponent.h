/******************************************************************/
/*!
\file      VideoComponent.h
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Feb 02, 2026
\brief     This file contains data for the video


Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#pragma once

#include "Component.h"

namespace ecs {

	class VideoComponent : public Component {

	public:

		utility::GUID videoGUID;

		bool playing = true;

		bool pause = false;

		bool loop = true;

		REFLECTABLE(VideoComponent, videoGUID, pause, loop);
	};

}


