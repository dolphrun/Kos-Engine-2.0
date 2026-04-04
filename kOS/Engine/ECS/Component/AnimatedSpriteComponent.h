
#ifndef ANIMSPRITE_H
#define ANIMSPRITE_H

#include "Component.h"

namespace ecs {

	class AnimatedSpriteComponent : public Component {

	public:
		std::vector<utility::GUID> spriteGUID{};
		glm::vec4 color{ 1.f,1.f,1.f,1.f };
		int currentFrame = 0;
		int framesPerSecond = 24;

		// NEW: UV cropping support for health bars and progress bars
		glm::vec2 uvMin{ 0.0f, 0.0f }; // Bottom-left UV coordinate (default: 0,0)
		glm::vec2 uvMax{ 1.0f, 1.0f }; // Top-right UV coordinate (default: 1,1)
		bool useCustomUV{ false }; // Enable custom UV coordinates
		bool isLooping{ false };
		REFLECTABLE(AnimatedSpriteComponent, spriteGUID, color, currentFrame, framesPerSecond, uvMin, uvMax, useCustomUV, isLooping);
	};

}

#endif ANIMSPRITE_H
