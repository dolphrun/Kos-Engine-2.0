#pragma once
#include "ScriptAdapter/TemplateSC.h"


// --- BOTTLE SCRIPT ---
class MainMenuBottleScript : public TemplateSC {
public:

    void Start() override;
    void Update() override;

    // Time
    float time;
    float maxTime;

    // Movement
    glm::vec3 startPos;
    glm::vec3 direction;
    float speed;

    // Rotation
    float rotationSpeed;
    float targetRotationZ;

    // Alignment
    float alignStartTime;
    float alignStartRotationZ;
    bool hasStoredStartRot;
   
    REFLECTABLE(MainMenuBottleScript)
};

// --- IMPLEMENTATION ---

inline void MainMenuBottleScript::Start() {

    time = 0.0f;

    // Must match camera
    maxTime = 1.5f;

    direction = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    speed = 4.2f;

    rotationSpeed = -360.0f;

    // Adjust this so logo faces camera
    targetRotationZ = 60.0f;

    // Start aligning near the end
    alignStartTime = maxTime - 0.1f;
    hasStoredStartRot = false;

    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    startPos = tf->LocalTransformation.position;

}

inline void MainMenuBottleScript::Update() {
    
    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    float dt = ecsPtr->m_GetDeltaTime();

    if (time >= maxTime) {
        tf->LocalTransformation.rotation.z = targetRotationZ;
        return;
    }

    time += dt;

    // Position
    glm::vec3 newPos = startPos + direction * speed * time;
    tf->LocalTransformation.position = newPos;

    float currentZ = tf->LocalTransformation.rotation.z;

    // Rotation
    if (time < alignStartTime) {

        // Free rolling
        currentZ += rotationSpeed * dt;
        tf->LocalTransformation.rotation.z = currentZ;

    }
    else {

        // Store starting rotation once
        if (!hasStoredStartRot) {
            alignStartRotationZ = currentZ;
            hasStoredStartRot = true;
        }

        float t = (time - alignStartTime) / (maxTime - alignStartTime);
        t = glm::clamp(t, 0.0f, 1.0f);

        // Smoothstep easing
        t = t * t * (3.0f - 2.0f * t);

        // Shortest path rotation
        float delta = targetRotationZ - alignStartRotationZ;
        while (delta > 180.f) delta -= 360.f;
        while (delta < -180.f) delta += 360.f;

        float finalZ = alignStartRotationZ + delta * t;

        tf->LocalTransformation.rotation.z = finalZ;
    }

}
