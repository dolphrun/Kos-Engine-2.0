#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include <glm/glm.hpp>
#include <cmath>
#include <cstdlib>

// --- CAMERA SCRIPT ---
class MainMenuHandBlockScript : public TemplateSC {
public:

    void Start() override;
    void Update() override;

    float time;
    float duration;

    glm::vec3 startPos;
    glm::vec3 targetPos;

    bool finished;

    // Set this manually
    glm::vec3 targetPosition = glm::vec3(5.52f, 10.5f, -3.62f);


    REFLECTABLE(MainMenuHandBlockScript)
};

// --- IMPLEMENTATION ---

inline void MainMenuHandBlockScript::Start() {
    time = 0.0f;
    duration = 1.65f; // tweak speed here

    finished = false;

    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    startPos = tf->LocalTransformation.position;
}

inline void MainMenuHandBlockScript::Update() {

    if (finished) return;

    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    float dt = ecsPtr->m_GetDeltaTime();
    time += dt;

    float t = time / duration;
    t = glm::clamp(t, 0.0f, 1.0f);

    // --- EASING (faster start, slower end) ---
    float easedT = 1.0f - pow(1.0f - t, 3.0f); // ease-out cubic

    // --- BASE LINEAR INTERPOLATION ---
    glm::vec3 pos = startPos + (targetPosition - startPos) * easedT;

    // --- ADD SLIGHT ARC (hand doesn't move perfectly straight) ---
    float arcHeight = 0.15f; // tweak
    pos.y += sin(t * 3.14159f) * arcHeight;

    // --- SUBTLE MICRO JITTER (very small) ---
    float noiseStrength = 0.007f;
    float noise = sin(time * 20.0f) * noiseStrength;

    pos.x += noise;
    pos.y += noise * 0.5f;

    tf->LocalTransformation.position = pos;

    if (t >= 1.0f) {
        finished = true;
    }
}
