#include "Config/pch.h"
#include "MeshColliderSystem.h"

namespace ecs {
    void MeshColliderSystem::Init() {
        onDeregister.Add([&](EntityID id) {
            MeshColliderComponent* meshCol = m_ecs.GetComponent<MeshColliderComponent>(id);
            if (!meshCol || !meshCol->shape) { return; }
            RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
            if (rb) { meshCol->shape = nullptr; }
            else {
                PxRigidStatic* actor = meshCol->actor ? static_cast<PxRigidStatic*>(meshCol->actor) : nullptr;
                PxShape* shape = static_cast<PxShape*>(meshCol->shape);
                if (actor) {
                    actor->detachShape(*shape);
                    if (actor->getNbShapes() == 0) {
                        PxScene* scene = actor->getScene();
                        if (scene) { scene->removeActor(*actor); }
                        actor->release();
                    }
                }
                PxGeometryHolder geomHolder = shape->getGeometry();
                if (geomHolder.getType() == PxGeometryType::eTRIANGLEMESH) {
                    PxTriangleMeshGeometry& triGeom = geomHolder.triangleMesh();
                    if (triGeom.triangleMesh) {
                        triGeom.triangleMesh->release();
                    }
                }
                else if (geomHolder.getType() == PxGeometryType::eCONVEXMESH) {
                    PxConvexMeshGeometry& convexGeom = geomHolder.convexMesh();
                    if (convexGeom.convexMesh) {
                        convexGeom.convexMesh->release();
                    }
                }
                shape->release();
                meshCol->shape = nullptr;
                meshCol->actor = nullptr;
            }
            });
    }

    void MeshColliderSystem::Update() {
        const auto& entities = m_entities.Data();
        for (EntityID id : entities) {
            TransformComponent* trans = m_ecs.GetComponent<TransformComponent>(id);
            NameComponent* name = m_ecs.GetComponent<NameComponent>(id);
            MeshColliderComponent* mesh = m_ecs.GetComponent<MeshColliderComponent>(id);
            MeshFilterComponent* meshFilter = m_ecs.GetComponent<MeshFilterComponent>(id);

            if (name->hide || !mesh || !meshFilter) { continue; }
            if (meshFilter->meshGUID.Empty()) { continue; }

            PxFilterData filter;
            filter.word0 = name->Layer;

            glm::vec3& scale = trans->LocalTransformation.scale;
            PxShape* shape = static_cast<PxShape*>(mesh->shape);

            if (!shape) {
                std::shared_ptr<R_Model> model = m_resourceManager.GetResource<R_Model>(meshFilter->meshGUID);
                if (!model || model->meshes.empty()) { continue; }

                std::vector<PxVec3> vertices;
                std::vector<PxU32> indices;
                PxU32 offset = 0;

                for (const auto& m : model->meshes) {
                    for (const auto& v : m.vertices) {
                        vertices.push_back(PxVec3{ v.Position.x, v.Position.y, v.Position.z });
                    }
                    for (const auto& i : m.indices) {
                        indices.push_back(i + offset);
                    }
                    offset += static_cast<PxU32>(m.vertices.size());
                }

                if (vertices.empty() || indices.empty()) { continue; }

                if (mesh->convex) {
                    PxConvexMesh* convexMesh = CreateConvexMesh(vertices);
                    if (!convexMesh) { continue; }
                    PxConvexMeshGeometry geometry{ convexMesh, PxMeshScale{ PxVec3{ scale.x, scale.y, scale.z } } };
                    shape = m_physicsManager.GetPhysics()->createShape(geometry, *m_physicsManager.GetDefaultMaterial(), true);
                } else {
                    PxTriangleMesh* triangleMesh = CreateTriangleMesh(vertices, indices);
                    if (!triangleMesh) { continue; }

                    PxTriangleMeshGeometry geometry{ triangleMesh, PxMeshScale{ PxVec3{ scale.x, scale.y, scale.z } } };
                    shape = m_physicsManager.GetPhysics()->createShape(geometry, *m_physicsManager.GetDefaultMaterial(), true);
                }

                if (!shape) { continue; }
                mesh->shape = shape;
            }

            PxGeometryHolder geometryHolder = shape->getGeometry();
            if (geometryHolder.getType() == PxGeometryType::eTRIANGLEMESH) {
                PxTriangleMeshGeometry& triangleGeometry = geometryHolder.triangleMesh();
                triangleGeometry.scale.scale = PxVec3{ scale.x, scale.y, scale.z };
                shape->setGeometry(triangleGeometry);
            }
            else if (geometryHolder.getType() == PxGeometryType::eCONVEXMESH) {
                PxConvexMeshGeometry& convexGeometry = geometryHolder.convexMesh();
                convexGeometry.scale.scale = PxVec3{ scale.x, scale.y, scale.z };
                shape->setGeometry(convexGeometry);
            }

            shape->setLocalPose(PxTransform{ PxVec3{ 0.0f, 0.0f, 0.0f } });
            ToPhysxIsTrigger(shape, mesh->isTrigger);
            shape->setSimulationFilterData(filter);
            shape->setQueryFilterData(filter);

            RigidbodyComponent* rb = m_ecs.GetComponent<RigidbodyComponent>(id);
            if (!rb) {
                PxRigidStatic* actor = mesh->actor ? static_cast<PxRigidStatic*>(mesh->actor) : nullptr;
                glm::vec3 pos = trans->WorldTransformation.position;
                glm::quat rot{ glm::radians(trans->WorldTransformation.rotation) };
                PxTransform pxTrans{ PxVec3{ pos.x, pos.y, pos.z }, PxQuat{ rot.x, rot.y, rot.z, rot.w } };
                if (!actor) {
                    actor = m_physicsManager.GetPhysics()->createRigidStatic(pxTrans);
                    actor->userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
                    m_physicsManager.GetScene()->addActor(*actor);
                    mesh->actor = actor;
                } else {
                    actor->setGlobalPose(pxTrans);
                }
                if (!IsShapeAttachedToActor(actor, shape)) {
                    actor->attachShape(*shape);
                }
            } else {
                mesh->actor = rb->actor;
            }
        }
    }

    bool MeshColliderSystem::IsShapeAttachedToActor(PxRigidActor* actor, PxShape* shape) {
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

    void MeshColliderSystem::LargeTrianglesSubdivision(std::vector<PxVec3>& vertices, std::vector<PxU32>& indices) {
        std::vector<PxU32> result;
        result.reserve(indices.size());

		std::unordered_map<uint64_t, PxU32> midpointCache;

        auto GetMidpoint = [&](PxU32 a, PxU32 b) -> PxU32 {
            if (a > b) { std::swap(a, b); }
            uint64_t key = (static_cast<uint64_t>(a) << 32) | b;
            auto it = midpointCache.find(key);
            if (it != midpointCache.end()) { return it->second; }
            PxVec3 mid = (vertices[a] + vertices[b]) * 0.5f;
            PxU32 i = static_cast<PxU32>(vertices.size());
            vertices.push_back(mid);
            midpointCache[key] = i;
            return i;
        };

        std::vector<std::array<PxU32, 3>> stack;
        stack.reserve(indices.size() / 3);

        for (size_t i = 0; i < indices.size(); i += 3) {
            stack.push_back({ indices[i], indices[i + 1], indices[i + 2] });
        }

		const float MAX_EDGE_LENGTH = 50.0f; 

        while (!stack.empty()) {
            auto [i0, i1, i2] = stack.back();
            stack.pop_back();

            const PxVec3& v0 = vertices[i0];
            const PxVec3& v1 = vertices[i1];
            const PxVec3& v2 = vertices[i2];

            float e0 = (v1 - v0).magnitudeSquared();
            float e1 = (v2 - v1).magnitudeSquared();
            float e2 = (v0 - v2).magnitudeSquared();

            float maxLen = std::max({ e0, e1, e2 });
            float threshold = MAX_EDGE_LENGTH * MAX_EDGE_LENGTH;

            if (maxLen <= threshold) {
                result.push_back(i0);
                result.push_back(i1);
                result.push_back(i2);
            } else {
                if (maxLen == e0) {
                    PxU32 m = GetMidpoint(i0, i1);
                    stack.push_back({ i0, m,  i2 });
                    stack.push_back({ m,  i1, i2 });
                }
                else if (maxLen == e1) {
                    PxU32 m = GetMidpoint(i1, i2);
                    stack.push_back({ i0, i1, m });
                    stack.push_back({ i0, m,  i2 });
                } else {
                    PxU32 m = GetMidpoint(i2, i0);
                    stack.push_back({ i0, i1, m });
                    stack.push_back({ m,  i1, i2 });
                }
            }
        }

        indices = std::move(result);
    }

    PxTriangleMesh* MeshColliderSystem::CreateTriangleMesh(std::vector<PxVec3> vertices, std::vector<PxU32> indices) {
        LargeTrianglesSubdivision(vertices, indices);

        PxTriangleMeshDesc meshDesc;
        meshDesc.points.count = static_cast<PxU32>(vertices.size());
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.points.data = vertices.data();

        meshDesc.triangles.count = static_cast<PxU32>(indices.size() / 3);
        meshDesc.triangles.stride = 3 * sizeof(PxU32);
        meshDesc.triangles.data = indices.data();

        PxDefaultMemoryOutputStream writeBuffer;
        PxTriangleMeshCookingResult::Enum result;
        bool status = PxCookTriangleMesh(m_physicsManager.GetPhysics()->getTolerancesScale(), meshDesc, writeBuffer, &result);

        if (!status) {
            LOGGING_ERROR("Failed to cook triangle mesh - Result: ", static_cast<int>(result));
            return nullptr;
        }

        PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        return m_physicsManager.GetPhysics()->createTriangleMesh(readBuffer);
    }

    PxConvexMesh* MeshColliderSystem::CreateConvexMesh(const std::vector<PxVec3>& vertices) {
        PxConvexMeshDesc meshDesc;
        meshDesc.points.count = static_cast<PxU32>(vertices.size());
        meshDesc.points.stride = sizeof(PxVec3);
        meshDesc.points.data = vertices.data();
        meshDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;

        PxDefaultMemoryOutputStream writeBuffer;
        PxConvexMeshCookingResult::Enum result;
        bool status = PxCookConvexMesh(m_physicsManager.GetPhysics()->getTolerancesScale(), meshDesc, writeBuffer, &result);

        if (!status) {
            LOGGING_ERROR("Failed to cook convex mesh - Result: ", static_cast<int>(result));
            return nullptr;
        }

        PxDefaultMemoryInputData readBuffer(writeBuffer.getData(), writeBuffer.getSize());
        return m_physicsManager.GetPhysics()->createConvexMesh(readBuffer);
    }
}