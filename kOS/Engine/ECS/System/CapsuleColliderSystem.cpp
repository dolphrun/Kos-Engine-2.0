#include "Config/pch.h"
#include "CapsuleColliderSystem.h"
#include "Physics/PhysicsManager.h"

namespace {
	constexpr float MINSIZE = 0.01f;
}

namespace ecs {
	void CapsuleColliderSystem::Init() {
		onDeregister.Add([&](EntityID id) {
			CapsuleColliderComponent* capsule = m_ecs.GetComponent<CapsuleColliderComponent>(id);
			if (!capsule || !capsule->shape) { return; }

			RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
			if (rb) { capsule->shape = nullptr; }
			else {
				PxRigidStatic* actor = capsule->actor ? static_cast<PxRigidStatic*>(capsule->actor) : nullptr;
				PxShape* shape = static_cast<PxShape*>(capsule->shape);
				if (actor) {
					actor->detachShape(*shape);
					if (actor->getNbShapes() == 0) {
						PxScene* scene = actor->getScene();
						if (scene) { scene->removeActor(*actor); }
						actor->release();
					}
				}
				shape->release();
				capsule->shape = nullptr;
				capsule->actor = nullptr;
			}
		});
	}

	void CapsuleColliderSystem::Update() {
		const auto& entities = m_entities.Data();
		for (EntityID id : entities) {
			TransformComponent* trans = m_ecs.GetComponent<TransformComponent>(id);
			NameComponent* name = m_ecs.GetComponent<NameComponent>(id);
			CapsuleColliderComponent* capsule = m_ecs.GetComponent<CapsuleColliderComponent>(id);

			if (name->hide || !capsule) { continue; }

			PxFilterData filter;
			filter.word0 = name->Layer;

			glm::vec3& scale = trans->WorldTransformation.scale;
			scale.x = glm::max(scale.x, MINSIZE);
			scale.y = glm::max(scale.y, MINSIZE);
			scale.z = glm::max(scale.z, MINSIZE);
			float radiusScale = 1.0f;
			float heightScale = 1.0f;

			switch (capsule->capsule.capsuleDirection) {
			case CapsuleDirection::X:
				radiusScale = glm::max(scale.y, scale.z);
				heightScale = scale.x;
				break;
			case CapsuleDirection::Y:
				radiusScale = glm::max(scale.x, scale.z);
				heightScale = scale.y;
				break;
			case CapsuleDirection::Z:
				radiusScale = glm::max(scale.x, scale.y);
				heightScale = scale.z;
				break;
			}
			float r = capsule->capsule.radius * radiusScale;
			float h = capsule->capsule.height * heightScale;
			h = glm::max(h, 2.0f * r);
			float halfHeight = h * 0.5f - r;
			PxShape* shape = static_cast<PxShape*>(capsule->shape);
			PxCapsuleGeometry geometry{ r, halfHeight };

			if (!shape) {
				shape = m_physicsManager.GetPhysics()->createShape(geometry, *m_physicsManager.GetDefaultMaterial(), true);
				capsule->shape = shape;
			}

			shape->setGeometry(geometry);

			PxQuat rot{ PxIdentity };
			switch (capsule->capsule.capsuleDirection) {
			case CapsuleDirection::X:
				rot = PxQuat(PxIdentity);
				break;
			case CapsuleDirection::Y:
				rot = PxQuat(PxHalfPi, PxVec3{ 0.0f, 0.0f, 1.0f });
				break;

			case CapsuleDirection::Z:
				rot = PxQuat(-PxHalfPi, PxVec3{ 0.0f, 1.0f, 0.0f });
				break;
			}

			glm::vec3 scaledCenter = capsule->capsule.center * scale;
			shape->setLocalPose(PxTransform{ PxVec3{ scaledCenter.x, scaledCenter.y, scaledCenter.z }, rot });
			ToPhysxIsTrigger(shape, capsule->isTrigger);
			shape->setSimulationFilterData(filter);
			shape->setQueryFilterData(filter);

			RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
			if (!rb) {
				PxRigidStatic* actor = capsule->actor ? static_cast<PxRigidStatic*>(capsule->actor) : nullptr;
				glm::vec3 pos = trans->WorldTransformation.position;
				glm::quat rot{ glm::radians(trans->WorldTransformation.rotation) };
				PxTransform pxTrans{ PxVec3{ pos.x, pos.y, pos.z }, PxQuat{ rot.x, rot.y, rot.z, rot.w } };
				if (!actor) {
					actor = m_physicsManager.GetPhysics()->createRigidStatic(pxTrans);
					actor->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
					m_physicsManager.GetScene()->addActor(*actor);
					capsule->actor = actor;
				} else {
					actor->setGlobalPose(pxTrans);
				}
				if (!IsShapeAttachedToActor(actor, shape)) {
					actor->attachShape(*shape);
				}
			} else {
				capsule->actor = rb->actor;
			}
		}
	}

	bool CapsuleColliderSystem::IsShapeAttachedToActor(PxRigidActor* actor, PxShape* shape) {
		if (!shape) { return false; }
		PxU32 nbShapes = actor->getNbShapes();
		std::vector<PxShape*> shapes(nbShapes);
		actor->getShapes(shapes.data(), nbShapes);
		for (PxU32 i = 0; i < nbShapes; ++i) {
			if (shapes[i] == shape) {
				return true;
			}
		}
		return false;
	}
}