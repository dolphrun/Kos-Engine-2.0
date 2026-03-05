/********************************************************************/
/*!
\file       BoxColliderComponent.h
\author		Toh Yu Heng 2301294
\par		t.yuheng@digipen.edu
\date		Oct 01 2025
\brief		Defines the BoxColliderComponent class, representing a 
            box-shaped collision volume used for physical interactions 
            or trigger detection within the ECS framework.

            This component holds shape data (size, position) and a 
            trigger flag to determine whether it participates in 
            physics simulation or acts purely as an event trigger.

            The PhysX shape pointer is stored as a void* for engine-
            level binding and managed by the PhysicsManager.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef BOXCOLLIDERCOMPONENT_H
#define BOXCOLLIDERCOMPONENT_H

#include "Component.h"
#include "Utility/Shapes.h"

namespace ecs {
	class BoxColliderComponent : public Component {
	public:
		bool isTrigger = false;
		utility::Box box;
		void* actor = nullptr;
		void* shape = nullptr;
		bool autoFit = false;
		REFLECTABLE(BoxColliderComponent, isTrigger, box)
	};
}

#endif