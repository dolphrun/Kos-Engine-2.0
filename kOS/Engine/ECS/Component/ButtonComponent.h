#ifndef BUTTONCOMPONENT_H
#define BUTTONCOMPONENT_H

#include "Component.h"

namespace ecs {
	class ButtonComponent : public Component {
	public:
		glm::vec4 normalColor{ 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 hoveredColor{ 0.8f, 0.8f, 0.8f, 1.0f };
		glm::vec4 pressedColor{ 0.6f, 0.6f, 0.6f, 1.0f };

		bool isHovered{ false };
		bool isPressed{ false };

		REFLECTABLE(ButtonComponent, normalColor, hoveredColor, pressedColor, isHovered, isPressed)
	};
}

#endif