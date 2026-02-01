/******************************************************************/
/*!
\file      SpriteComponent.h
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 02, 2024
\brief     This file contains data for the sprite component


Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#ifndef SPRITE_H
#define SPRITE_H

#include "Component.h"

namespace ecs {

	class SpriteComponent : public Component {

	public:
		utility::GUID spriteGUID{};
		glm::vec4 color{ 1.f,1.f,1.f,1.f };

		// NEW: UV cropping support for health bars and progress bars
		glm::vec2 uvMin{ 0.0f, 0.0f }; // Bottom-left UV coordinate (default: 0,0)
		glm::vec2 uvMax{ 1.0f, 1.0f }; // Top-right UV coordinate (default: 1,1)
		bool useCustomUV{ false }; // Enable custom UV coordinates
		REFLECTABLE(SpriteComponent, spriteGUID, color, uvMin, uvMax, useCustomUV);
	};

}

#endif SPRITE_H
