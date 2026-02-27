#ifndef MESHCOLLIDERCOMPONENT_H
#define	MESHCOLLIDERCOMPONENT_H

#include "Component.h"

namespace ecs {
	class MeshColliderComponent : public Component {
	public:
		bool isTrigger = false;
		bool convex = false;
		void* actor = nullptr;
		void* shape = nullptr;

		REFLECTABLE(MeshColliderComponent, isTrigger, convex)
	};
}

#endif