#include "Config/pch.h"
#include "BoxColliderSystem.h"
#include "Physics/PhysicsManager.h"

namespace {
	constexpr float MINSIZE = 0.01f;
}

namespace ecs {
	void BoxColliderSystem::Init() {
		onDeregister.Add([&](EntityID id) {
			BoxColliderComponent* box = m_ecs.GetComponent<BoxColliderComponent>(id);
			if (!box || !box->shape) { return; }

			RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
			if (rb) { box->shape = nullptr; }
			else {
				PxRigidStatic* actor = box->actor ? static_cast<PxRigidStatic*>(box->actor) : nullptr;
				PxShape* shape = static_cast<PxShape*>(box->shape);
				if (actor) {
					actor->detachShape(*shape);
					if (actor->getNbShapes() == 0) {
						PxScene* scene = actor->getScene();
						if (scene) { scene->removeActor(*actor); }
						actor->release();
					}
				}
				shape->release();
				box->shape = nullptr;
				box->actor = nullptr;
			}
		});
	}

	void BoxColliderSystem::Update() {
		const auto& entities = m_entities.Data();
		for (EntityID id : entities) {
			TransformComponent* trans = m_ecs.GetComponent<TransformComponent>(id);
			NameComponent* name = m_ecs.GetComponent<NameComponent>(id);
			BoxColliderComponent* box = m_ecs.GetComponent<BoxColliderComponent>(id);

			if (name->hide || !box) { continue; }

			PxFilterData filter;
			filter.word0 = name->Layer;

			glm::vec3& scale = trans->LocalTransformation.scale;
			scale.x = glm::max(scale.x, MINSIZE);
			scale.y = glm::max(scale.y, MINSIZE);
			scale.z = glm::max(scale.z, MINSIZE);

			glm::vec3 halfExtents = box->box.size * scale * 0.5f;
			PxShape* shape = static_cast<PxShape*>(box->shape);
			PxBoxGeometry geometry{ halfExtents.x, halfExtents.y, halfExtents.z };
			
			if (!shape) {
				MeshFilterComponent* meshFilter = m_ecs.GetComponent<MeshFilterComponent>(id);
				if (meshFilter && !meshFilter->meshGUID.Empty() && !box->autoFit) {
					std::shared_ptr<R_Model> model = m_resourceManager.GetResource<R_Model>(meshFilter->meshGUID);
					if (model && !model->meshes.empty()) {
						glm::vec3 minBound{ FLT_MAX };
						glm::vec3 maxBound{ -FLT_MAX };
						for (const auto& m : model->meshes) {
							for (const auto& v : m.vertices) {
								minBound = glm::min(minBound, v.Position);
								maxBound = glm::max(maxBound, v.Position);
							}
						}
						box->box.center = (minBound + maxBound) * 0.5f;
						box->box.size = maxBound - minBound;
						box->autoFit = true;
						halfExtents = box->box.size * scale * 0.5f;
						geometry = PxBoxGeometry{ halfExtents.x, halfExtents.y, halfExtents.z };
					}
				}
				shape = m_physicsManager.GetPhysics()->createShape(geometry, *m_physicsManager.GetDefaultMaterial(), true);
				box->shape = shape;
			}

			shape->setGeometry(geometry);
			glm::vec3 scaledCenter = box->box.center * scale;
			shape->setLocalPose(PxTransform{ PxVec3{ scaledCenter.x, scaledCenter.y, scaledCenter.z } });
			ToPhysxIsTrigger(shape, box->isTrigger);
			shape->setSimulationFilterData(filter);
			shape->setQueryFilterData(filter);
			
			box->box.bounds.center = trans->WorldTransformation.position + scaledCenter;
			box->box.bounds.extents = halfExtents;
			box->box.bounds.size = box->box.size * scale;
			box->box.bounds.min = box->box.bounds.center - box->box.bounds.extents;
			box->box.bounds.max = box->box.bounds.center + box->box.bounds.extents;

			RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
			if (!rb) {
				PxRigidStatic* actor = box->actor ? static_cast<PxRigidStatic*>(box->actor) : nullptr;
				glm::vec3 pos = trans->WorldTransformation.position;
				glm::quat rot{ glm::radians(trans->WorldTransformation.rotation) };
				PxTransform pxTrans{ PxVec3{ pos.x, pos.y, pos.z }, PxQuat{ rot.x, rot.y, rot.z, rot.w } };
				if (!actor) {
					actor = m_physicsManager.GetPhysics()->createRigidStatic(pxTrans);
					actor->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
					m_physicsManager.GetScene()->addActor(*actor);
					box->actor = actor;
				} else { 
					actor->setGlobalPose(pxTrans); 
				}
				if (!IsShapeAttachedToActor(actor, shape)) { 
					actor->attachShape(*shape); 
				}
			} else { 
				box->actor = rb->actor; 
			}
		}
	}

	bool BoxColliderSystem::IsShapeAttachedToActor(PxRigidActor* actor, PxShape* shape) {
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