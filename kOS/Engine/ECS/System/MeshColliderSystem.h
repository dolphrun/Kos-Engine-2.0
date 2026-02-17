#ifndef MESHCOLLIDERSYSTEM_H
#define MESHCOLLIDERSYSTEM_H

#include "ECS/ECS.h"
#include "System.h"

namespace ecs {
	class MeshColliderSystem : public ISystem {
	public:
		using ISystem::ISystem;
		void Init() override;
		void Update() override;
		REFLECTABLE(MeshColliderSystem)
	private:
		bool IsShapeAttachedToActor(PxRigidActor*, PxShape*);
		void LargeTrianglesSubdivision(std::vector<PxVec3>&, std::vector<PxU32>&);
		PxTriangleMesh* CreateTriangleMesh(std::vector<PxVec3>, std::vector<PxU32>);
		PxConvexMesh* CreateConvexMesh(const std::vector<PxVec3>&);
	};
}

#endif