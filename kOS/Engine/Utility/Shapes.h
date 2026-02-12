/********************************************************************/
/*!
\file       Shapes.h
\authors	Toh Yu Heng 2301294
\par		t.yuheng@digipen.edu
\date		Oct 01 2025
\brief		Defines basic geometric shape structures used in the
			physics and rendering systems, including their spatial
			properties and bounding box data.

			These shape definitions serve as lightweight containers
			for describing collision geometry, spatial queries, and
			visualization primitives.

			Supported shapes:
			- AABB (Axis-Aligned Bounding Box)
			- Box
			- Capsule
			- Sphere

			Each shape includes position, size, and dimension data,
			and is integrated with the engine’s reflection system
			using the REFLECTABLE macro for editor and serialization
			support.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef SHAPES_H
#define SHAPES_H

#include "Config/pch.h" 
#include "Reflection/Reflection.h"

enum class CapsuleDirection { X = 0, Y, Z };

namespace utility {
	struct AABB {
		glm::vec3 center{ 0.0f };
		glm::vec3 extents{ 0.5f };
		glm::vec3 min{ -0.5f };
		glm::vec3 max{ 0.5f };
		glm::vec3 size{ 1.0f };
	};

	struct Box {
		glm::vec3 center{ 0.0f };
		glm::vec3 size{ 1.0f };
		AABB bounds;
		REFLECTABLE(Box, center, size)
	};

	struct Capsule {
		glm::vec3 center{ 0.0f };
		float radius{ 0.5f };
		float height{ 2.0f };
		CapsuleDirection capsuleDirection{ CapsuleDirection::Y };
		REFLECTABLE(Capsule, center, radius, height, capsuleDirection)
	};

	struct Sphere {
		glm::vec3 center{ 0.0f };
		float radius{ 0.5f };
		REFLECTABLE(Sphere, center, radius)
	};

	struct Plane {
		glm::vec3 normal{ 0.f,1.f,0.f };
		float distance;
		Plane() = default;
		Plane(const glm::vec3& p, const glm::vec3& n)
			: normal(glm::normalize(n)),
			distance(glm::dot(normal, p)) {}
		REFLECTABLE(Plane, normal, distance);
	};
	struct Fustrum {
		union {
			struct {
				Plane nearFace;
				Plane farFace;
				Plane rightFace;
				Plane leftFace;
				Plane topFace;
				Plane bottomFace;
			};
			Plane planes[6];
		};
		Fustrum() :planes{} {}
	};
}

#endif