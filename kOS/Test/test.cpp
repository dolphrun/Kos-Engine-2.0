/******************************************************************/
/*!
\file      test.cpp
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 02, 2025
\brief     This file contains defination of the test cases for ecs components
		   and some math utility functions.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/
#include <gtest/gtest.h>
#include "common.h"
#include "ECS/ECS.h"
#include "Scene/SceneManager.h"
#include "Utility/MathUtility.h"
#include "glm/gtx/euler_angles.hpp"
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/string_cast.hpp>

// New includes
#include "Events/Delegate.h"
#include "ECS/SparseSet.h"

// Includes for Smoke Test dependencies
#include "Debugging/Performance.h"
#include "Graphics/GraphicsManager.h"
#include "Resources/ResourceManager.h"
#include "Inputs/Input.h"
#include "Physics/PhysicsManager.h"
#include "Scripting/ScriptManager.h"
#include "Audio/AudioManager.h"
#include "Scene/SceneData.h" 

using namespace ecs;

SERIALIZE_DESERIALIZE_COMPARE_TEST(TransformComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(NameComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(SpriteComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(CameraComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(AudioComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(TextComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(CanvasRendererComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(MeshFilterComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(SkinnedMeshRendererComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(AnimatorComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(LightComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(ScriptComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(BoxColliderComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(SphereColliderComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(RigidbodyComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(OctreeGeneratorComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(PathfinderComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(PathfinderTargetComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(CubeRendererComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(SphereRendererComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(ParticleComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(MaterialComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(ButtonComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(AudioListenerComponent)
SERIALIZE_DESERIALIZE_COMPARE_TEST(CapsuleColliderComponent)
// Add more component tests as needed


//TEST(Scene, CreateScene) {
//    scenes::SceneManager sm;
//	EXPECT_TRUE(sm.ImmediateLoadScene("Test Scene"));
//
//	sm.ImmediateClearScene("Test Scene");
//}
//
//
//TEST(Entity, SpawnMaxEntities) {
//	auto* ecs =ComponentRegistry::GetECSInstance();
//	m_ecs.RegisterComponent<TransformComponent>();
//	m_ecs.RegisterComponent<NameComponent>();
//
//	auto* sm = scenes::SceneManager::m_GetInstance();
//	EXPECT_TRUE(sm->ImmediateLoadScene("Test Scene"));
//
//	const int numEntities = ecs::MaxEntity - 10;
//	
//	std::vector<ecs::EntityID> entityIDs;
//	entityIDs.reserve(numEntities);
//	for (int i = 0; i < numEntities; ++i) {
//		ecs::EntityID newEntity = m_ecs.CreateEntity("Test Scene");
//		entityIDs.push_back(newEntity);
//	}
//	// Verify that all entities were created successfully
//	for (const auto& id : entityIDs) {
//		EXPECT_TRUE(m_ecs.IsValidEntity(id));
//	}
//	// Clean up by destroying the created entities
//	for (const auto& id : entityIDs) {
//		m_ecs.DeleteEntity(id);
//	}
//
//	sm->ImmediateClearScene("Test Scene");
//}



TEST(Math, RandomDecomposeTRS) {
    constexpr int NUM_TESTS = 100;
    constexpr float EPS_POS = 0.0001f;
    constexpr float EPS_SCALE = 0.0001f;
    constexpr float EPS_ROT = 0.0001f;
    
	RandomizeComponents<glm::vec3> randomGen;

    for (int i = 0; i < NUM_TESTS; ++i) {
        glm::vec3 pos;
        glm::vec3 scale(1.f,2.f,5.f); // avoid zero scale
        glm::vec3 eulerRot;

		randomGen(pos);
		randomGen(eulerRot);

        // Build rotation quaternion
        glm::quat qx = glm::angleAxis(glm::radians(eulerRot.x), glm::vec3(1, 0, 0));
        glm::quat qy = glm::angleAxis(glm::radians(eulerRot.y), glm::vec3(0, 1, 0));
        glm::quat qz = glm::angleAxis(glm::radians(eulerRot.z), glm::vec3(0, 0, 1));
        glm::quat q = qz * qy * qx;

        // Build transformation matrix
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * glm::toMat4(q) * glm::scale(glm::mat4(1.0f), scale);

        // Decompose
        glm::vec3 decPos, decRot, decScale;
        utility::DecomposeMtxIntoTRS(transform, decPos, decRot, decScale);

        // Compare position
        EXPECT_TRUE(glm::all(glm::epsilonEqual(pos, decPos, EPS_POS)))
            << "Position mismatch: " << glm::to_string(pos) << " vs " << glm::to_string(decPos);

        // Compare scale
        EXPECT_TRUE(glm::all(glm::epsilonEqual(scale, decScale, EPS_SCALE)))
            << "Scale mismatch: " << glm::to_string(scale) << " vs " << glm::to_string(decScale);

        // Compare rotation via quaternions
        glm::quat decQuat = glm::quat(glm::radians(decRot));
        float dot = glm::dot(q, decQuat);
        EXPECT_NEAR(std::abs(dot), 1.0f, EPS_ROT)
            << "Rotation mismatch: " << glm::to_string(eulerRot) << " vs " << glm::to_string(decRot);
    }
}


// Delegate Tests
TEST(DelegateTest, SubscriptionAndInvoke) {
    Delegate<int> del;
    int value = 0;
    
    // Subscribe
    auto id = del.Add([&value](int v) { value = v; });
    
    // Invoke
    del.Invoke(42);
    EXPECT_EQ(value, 42);
    
    // Invoke again
    del.Invoke(100);
    EXPECT_EQ(value, 100);
}

TEST(DelegateTest, MultipleSubscribers) {
    Delegate<int> del;
    int count = 0;
    
    del.Add([&count](int) { count++; });
    del.Add([&count](int) { count++; });
    
    del.Invoke(1);
    EXPECT_EQ(count, 2);
}

TEST(DelegateTest, Unsubscribe) {
    Delegate<int> del;
    int value = 0;
    
    auto id = del.Add([&value](int v) { value = v; });
    
    del.Invoke(10);
    EXPECT_EQ(value, 10);
    
    del.Remove(id);
    del.Invoke(20);
    EXPECT_EQ(value, 10); // Should remain unchanged
}


// SparseSet Tests
TEST(SparseSetTest, SetAndGet) {
    ecs::SparseSet<int> sparseSet;
    ecs::EntityID entity = 100;
    int value = 42;
    
    // Set
    sparseSet.Set(entity, value);
    
    // Get
    int* retrieved = sparseSet.Get(entity);
    ASSERT_NE(retrieved, nullptr);
    EXPECT_EQ(*retrieved, value);
    
    // Const Get
    const auto& constSet = sparseSet;
    const int* constRetrieved = constSet.Get(entity);
    ASSERT_NE(constRetrieved, nullptr);
    EXPECT_EQ(*constRetrieved, value);
}

TEST(SparseSetTest, NonExistentEntity) {
    ecs::SparseSet<int> sparseSet;
    EXPECT_EQ(sparseSet.Get(999), nullptr);
    EXPECT_FALSE(sparseSet.ContainsEntity(999));
}

TEST(SparseSetTest, ContainsEntity) {
    ecs::SparseSet<int> sparseSet;
    ecs::EntityID entity = 1;
    sparseSet.Set(entity, 10);
    EXPECT_TRUE(sparseSet.ContainsEntity(entity));
    EXPECT_FALSE(sparseSet.ContainsEntity(2));
}

TEST(SparseSetTest, Delete) {
    ecs::SparseSet<int> sparseSet;
    ecs::EntityID entity = 50;
    sparseSet.Set(entity, 123);
    
    EXPECT_TRUE(sparseSet.ContainsEntity(entity));
    
    sparseSet.Delete(entity);
    
    EXPECT_FALSE(sparseSet.ContainsEntity(entity));
    EXPECT_EQ(sparseSet.Get(entity), nullptr);
}

TEST(SparseSetTest, SizeAndClear) {
    ecs::SparseSet<int> sparseSet;
    ecs::EntityID e1 = 1;
    ecs::EntityID e2 = 2;
    sparseSet.Set(e1, 10);
    sparseSet.Set(e2, 20);

    EXPECT_EQ(sparseSet.Size(), 2);

    sparseSet.Clear();
    EXPECT_EQ(sparseSet.Size(), 0);
}


TEST(SmokeTest, Initialization) {
    // Instantiate dependencies
    Peformance perf;
    GraphicsManager graphics;
    ResourceManager resourceManager;
    Input::InputSystem inputSystem;
    physics::PhysicsManager physicsManager;
    audio::AudioManager audioManager;

    // Instantiate ECS
    ecs::ECS ecsSystem(perf, graphics, resourceManager, inputSystem, physicsManager, audioManager);
    
    // Load ECS components (required for CreateEntity to work if it adds default components like Transform/Name)
    ecsSystem.Load();

    // Setup a dummy scene
    std::string sceneName = "SmokeTestScene";
    SceneData sceneData;
    sceneData.sceneName = sceneName;
    ecsSystem.AddScene(sceneName, sceneData);

    // Create Entity
    ecs::EntityID entity = ecsSystem.CreateEntity(sceneName);
    
    //Verify Entity exists
    EXPECT_TRUE(ecsSystem.IsValidEntity(entity));
    EXPECT_TRUE(ecsSystem.HasComponent<ecs::TransformComponent>(entity));
    EXPECT_TRUE(ecsSystem.HasComponent<ecs::NameComponent>(entity));
    
    // Cleanup
    ecsSystem.DeleteEntity(entity);
    ecsSystem.EndFrame();
    EXPECT_FALSE(ecsSystem.IsValidEntity(entity));
}



