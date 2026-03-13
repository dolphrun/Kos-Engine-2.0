/********************************************************************/
/*!
\file		PhysicsManager.cpp
\author		Toh Yu Heng 2301294
\par		t.yuheng@digipen.edu
\date		Oct 01 2025
\brief		This file defines the implementation of the PhysicsManager 
			class responsible for initializing, updating, and shutting 
			down the physics simulation using NVIDIA PhysX.

			The PhysicsManager handles the creation and management of 
			the PhysX foundation, physics instance, scene, dispatcher, 
			materials, and character controller manager. 

			It also provides interfaces for applying forces and torques 
			to entities with Rigidbody components in the ECS system, 
			allowing for physical interactions and motion simulation.

			Key functionalities:
			- Initialization of the PhysX SDK and scene
			- Fixed timestep physics simulation update
			- Safe release of PhysX resources during shutdown
			- Force and torque application to rigid bodies

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "PhysicsManager.h"

namespace physics {
	void PhysicsManager::Init() {
		m_foundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_allocator, m_errorCallback);
		PX_ASSERT(m_foundation);

		m_physics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_foundation, PxTolerancesScale{});
		PX_ASSERT(m_physics);

		PxSceneDesc sceneDesc{ m_physics->getTolerancesScale() };
		sceneDesc.gravity = PxVec3{ 0.0f, -29.81f, 0.0f };
		sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;                    
		sceneDesc.flags |= PxSceneFlag::eENABLE_PCM;
		m_cpuDispatcher = PxDefaultCpuDispatcherCreate(2);
		sceneDesc.cpuDispatcher = m_cpuDispatcher;
		sceneDesc.filterShader = ToPhysxCustomFilter;

		physicslayer::PhysicsFilterData physicsFilterData;
		physicsFilterData.layerSystem = &layers;
		sceneDesc.filterShaderData = &physicsFilterData;
		sceneDesc.filterShaderDataSize = sizeof(physicslayer::PhysicsFilterData);
		
		m_eventCallback = new PhysicsEventCallback();
		sceneDesc.simulationEventCallback = m_eventCallback;

		m_scene = m_physics->createScene(sceneDesc);
		PX_ASSERT(m_scene);

		m_defaultMaterial = m_physics->createMaterial(0.0f, 0.0f, 0.0f);
		PX_ASSERT(m_defaultMaterial);

		m_controllerManager = PxCreateControllerManager(*m_scene);
		PX_ASSERT(m_controllerManager);

		layers.LoadCollisionLayer();
	}

	void PhysicsManager::Shutdown() {
		if (m_eventCallback) {
			m_eventCallback->m_activeCollisions.clear();
			m_eventCallback->m_activeTriggers.clear();
		}

		if (m_controllerManager) {
			m_controllerManager->release();
			m_controllerManager = nullptr;
		}

		if (m_defaultMaterial) {
			m_defaultMaterial->release();
			m_defaultMaterial = nullptr;
		}

		if (m_scene) {
			m_scene->release();
			m_scene = nullptr;
		}

		if (m_eventCallback) {
			delete m_eventCallback;
			m_eventCallback = nullptr;
		}

		if (m_cpuDispatcher) {
			m_cpuDispatcher->release();
			m_cpuDispatcher = nullptr;
		}

		if (m_physics) {
			m_physics->release();
			m_physics = nullptr;
		}

		if (m_foundation) {
			m_foundation->release();
			m_foundation = nullptr;
		}
	}

	void PhysicsManager::Update(float deltaTime) {
		if (!m_scene) { return; }
		m_accumulator += deltaTime;
		if (m_accumulator > m_maximumDeltaTIme) { m_accumulator = m_maximumDeltaTIme; }
		m_frameCount = 0;
		while (m_accumulator >= m_fixedDeltaTime) {
			m_scene->simulate(m_fixedDeltaTime);
			m_scene->fetchResults(true);
			if (m_eventCallback) { 
				m_eventCallback->FlushEvents();
				m_eventCallback->ProcessCollisionStay();
				m_eventCallback->ProcessTriggerStay();
			}
			m_accumulator -= m_fixedDeltaTime;
			++m_frameCount;
		}
	}

	void PhysicsManager::AddForce(void* actor, const glm::vec3& force, ForceMode mode) {
		if (std::isnan(force.x) || std::isnan(force.y) || std::isnan(force.z) || actor == NULL) { return; }
		PxRigidDynamic* rb = static_cast<PxRigidDynamic*>(actor);
		if (!rb->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC)) {
			rb->addForce(PxVec3{ force.x, force.y, force.z }, ToPhysxForceMode(mode));
		}
	}

	void PhysicsManager::AddTorque(void* actor, const glm::vec3& torque, ForceMode mode) {
		PxRigidDynamic* rb = static_cast<PxRigidDynamic*>(actor);
		if (!rb->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC)) {
			rb->addTorque(PxVec3{ torque.x, torque.y, torque.z }, ToPhysxForceMode(mode));
		}
	}

	bool PhysicsManager::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit, void* actorToIgnore) {
		if (!m_scene) { return false; }

		PxVec3 pxOrigin{ origin.x, origin.y, origin.z };
		PxVec3 pxDirection{ direction.x, direction.y, direction.z };
		pxDirection.normalize();

		PxRaycastBuffer hit;
		PxRigidActor* ignoredActor = static_cast<PxRigidActor*>(actorToIgnore);

		struct IgnoreActorFilter : public PxQueryFilterCallback {
			PxRigidActor* ignore;
			IgnoreActorFilter(PxRigidActor* actor) : ignore(actor) {}

			PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags) override {
				if (actor == ignore) { return PxQueryHitType::eNONE; }
				return PxQueryHitType::eBLOCK;
			}

			PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override {
				PX_UNUSED(filterData);
				PX_UNUSED(hit);
				PX_UNUSED(shape);
				PX_UNUSED(actor);
				return PxQueryHitType::eBLOCK;
			}
		};

		PxQueryFilterData filterData{ PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER };
		IgnoreActorFilter filterCallback{ ignoredActor };

		bool isHit = m_scene->raycast(pxOrigin, pxDirection, maxDistance, hit, PxHitFlag::eDEFAULT, filterData, ignoredActor ? &filterCallback : nullptr);
		if (isHit && hit.hasBlock) {
			outHit.point = glm::vec3{ hit.block.position.x, hit.block.position.y, hit.block.position.z };
			outHit.normal = glm::vec3{ hit.block.normal.x, hit.block.normal.y, hit.block.normal.z };
			outHit.distance = hit.block.distance;
			if (hit.block.actor && hit.block.actor->userData) { outHit.entityID = reinterpret_cast<unsigned int>(hit.block.actor->userData); }
			m_debugRays.push_back({ origin, outHit.point });
			return true;
		}
		m_debugRays.push_back({ origin, origin + glm::normalize(direction) * maxDistance });
		return false;
	}

	bool PhysicsManager::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit, const std::vector<void*>& actorsToIgnore) {
		if (!m_scene) { return false; }

		PxVec3 pxOrigin{ origin.x, origin.y, origin.z };
		PxVec3 pxDirection{ direction.x, direction.y, direction.z };
		pxDirection.normalize();

		struct MultiIgnoreFilter : public PxQueryFilterCallback {
			const std::vector<void*>& ignoreList;
			MultiIgnoreFilter(const std::vector<void*>& list) : ignoreList(list) {}

			PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape,
				const PxRigidActor* actor, PxHitFlags& queryFlags) override {
				for (void* ignored : ignoreList) {
					if (actor == static_cast<PxRigidActor*>(ignored)) { return PxQueryHitType::eNONE; }
				}
				return PxQueryHitType::eBLOCK;
			}

			PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit,
				const PxShape* shape, const PxRigidActor* actor) override {
				return PxQueryHitType::eBLOCK;
			}
		};

		PxRaycastBuffer hit;
		PxQueryFilterData filterData{ PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER };
		MultiIgnoreFilter filterCallback{ actorsToIgnore };

		bool isHit = m_scene->raycast(pxOrigin, pxDirection, maxDistance, hit, PxHitFlag::eDEFAULT, filterData, actorsToIgnore.empty() ? nullptr : &filterCallback);

		if (isHit && hit.hasBlock) {
			outHit.point = { hit.block.position.x, hit.block.position.y, hit.block.position.z };
			outHit.normal = { hit.block.normal.x,   hit.block.normal.y,   hit.block.normal.z };
			outHit.distance = hit.block.distance;
			if (hit.block.actor && hit.block.actor->userData) {
				outHit.entityID = reinterpret_cast<unsigned int>(hit.block.actor->userData);
			}

			m_debugRays.push_back({ origin, outHit.point });
			return true;
		}

		m_debugRays.push_back({ origin, origin + glm::normalize(direction) * maxDistance });
		return false;
	}

	bool PhysicsManager::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit, int ignoredLayerMask, void* actorToIgnore) {
		if (!m_scene) { return false; }

		PxVec3 pxOrigin{ origin.x, origin.y, origin.z };
		PxVec3 pxDirection{ direction.x, direction.y, direction.z };
		pxDirection.normalize();

		struct LayerFilter : public PxQueryFilterCallback {
			int mask;
			PxRigidActor* ignore;
			LayerFilter(int m, PxRigidActor* a) : mask(m), ignore(a) {}
			PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags&) override {
				int layer = static_cast<int>(shape->getQueryFilterData().word0);
				if (mask & (1 << layer)) { return PxQueryHitType::eNONE; }
				if (ignore && actor == ignore) { return PxQueryHitType::eNONE; }
				return PxQueryHitType::eBLOCK;
			}
			PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override {
				return PxQueryHitType::eBLOCK;
			}
		};

		PxRaycastBuffer hit;
		PxQueryFilterData filterData{ PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER };
		LayerFilter cb{ ignoredLayerMask, static_cast<PxRigidActor*>(actorToIgnore) };
		m_scene->raycast(pxOrigin, pxDirection, maxDistance, hit, PxHitFlag::eDEFAULT, filterData, &cb);
		return ResolveHit(hit, origin, direction, maxDistance, outHit, m_debugRays);
	}

	bool PhysicsManager::Raycast(const glm::vec3& origin, const glm::vec3& direction, float maxDistance, RaycastHit& outHit, int ignoredLayerMask, const std::vector<void*>& actorsToIgnore) {
		if (!m_scene) { return false; }

		PxVec3 pxOrigin{ origin.x,    origin.y,    origin.z };
		PxVec3 pxDirection{ direction.x, direction.y, direction.z };
		pxDirection.normalize();

		struct LayerFilter : public PxQueryFilterCallback {
			int mask;
			const std::vector<void*>& ignoreList;
			LayerFilter(int m, const std::vector<void*>& l) : mask(m), ignoreList(l) {}
			PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags&) override {
				int layer = static_cast<int>(shape->getQueryFilterData().word0);
				if (mask & (1 << layer)) { return PxQueryHitType::eNONE; }
				for (void* ignored : ignoreList) {
					if (actor == static_cast<const PxRigidActor*>(ignored)) { return PxQueryHitType::eNONE; }
				}
				return PxQueryHitType::eBLOCK;
			}
			PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit, const PxShape* shape, const PxRigidActor* actor) override {
				return PxQueryHitType::eBLOCK;
			}
		};

		PxRaycastBuffer hit;
		PxQueryFilterData filterData{ PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC | PxQueryFlag::ePREFILTER };
		LayerFilter cb{ ignoredLayerMask, actorsToIgnore };

		m_scene->raycast(pxOrigin, pxDirection, maxDistance, hit, PxHitFlag::eDEFAULT, filterData, &cb);
		return ResolveHit(hit, origin, direction, maxDistance, outHit, m_debugRays);
	}

	std::vector<int> PhysicsManager::OverlapSphere(const glm::vec3& center, float radius) {
		std::vector<int> result;
		if (!m_scene) { return result; }
		PxSphereGeometry sphereGeom{ radius };
		PxTransform pos{ PxVec3{ center.x, center.y, center.z } };
		PxOverlapBuffer overlapBuffer;
		PxOverlapHit overlapHit[32];
		overlapBuffer = PxOverlapBuffer(overlapHit, 32);
		PxQueryFilterData filterData{ PxQueryFlag::eSTATIC | PxQueryFlag::eDYNAMIC };
		if (m_scene->overlap(sphereGeom, pos, overlapBuffer, filterData)) {
			for (PxU32 i = 0; i < overlapBuffer.getNbAnyHits(); ++i) {
				const PxOverlapHit& hit = overlapBuffer.getAnyHit(i);
				if (hit.actor && hit.actor->userData) {
					int id = static_cast<int>(reinterpret_cast<uintptr_t>(hit.actor->userData));
					result.push_back(id);
				}
			}
		}
		m_debugSpheres.push_back({ center, radius });
		return result;
	}
}