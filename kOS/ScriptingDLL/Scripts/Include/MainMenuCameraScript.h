#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include <glm/glm.hpp>
#include <cmath>
#include <cstdlib>


// --- PERLIN NOISE FUNCTIONS ---
inline float PerlinNoise1D(float x) {
    int xi = (int)floor(x) & 255;
    float xf = x - floor(x);
    float u = xf * xf * (3.0f - 2.0f * xf);

    auto hash = [](int n) { return (n * 16807) % 2147483647; };
    float g1 = (hash(xi) % 1000 / 1000.0f) * 2.0f - 1.0f;
    float g2 = (hash(xi + 1) % 1000 / 1000.0f) * 2.0f - 1.0f;

    return g1 + u * (g2 - g1);
}

inline float PerlinOctave1D(float x, int octaves, float persistence = 0.5f) {
    float total = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += PerlinNoise1D(x * frequency) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= 2.0f;
    }

    return total / maxValue;
}

// --- CAMERA SCRIPT ---
class MainMenuCameraScript : public TemplateSC {
public:

    void Start() override;
    void Update() override;

    // --- TIME CONTROL ---
    float time;
    float maxTime;           // total roll duration
    bool shakeTriggered;     // has the end shake been triggered?
    float endTimer;
    float fadeDelay;

    // --- BASE MOTION ---
    glm::vec3 startPos;
    glm::vec3 direction;
    float speed;

    // --- PERLIN BUMP SETTINGS ---
    int octaves;
    float vertScale;
    float horizScale;
    float rotScale;
    float vertSpeed;
    float horizSpeed;
    float rotSpeed;

    // --- MICRO-IMPACT SETTINGS ---
    float impactTimer;
    float impactInterval;
    float impactDuration;
    float impactAmplitude;
    glm::vec3 impactOffset;

    // --- SHAKE SETTINGS ---
    float cameraShakeIntensity;
    float cameraShakeDuration;
    float cameraShakeElapsed;
    glm::vec3 cameraShakeOriginalPos;
    bool isCameraShaking;

    bool fadeTriggered;

    void CameraShake(float intensity, float duration);

    utility::GUID fadeToBlackGO;
    ecs::EntityID fadeToBlackGOID;

    utility::GUID next_Scene;

    bool moveToNextScene = false;

    REFLECTABLE(MainMenuCameraScript, fadeToBlackGO, next_Scene)
};

#include "FadeTransition.h"

// --- IMPLEMENTATION ---

inline void MainMenuCameraScript::Start() {
    ecsPtr->SetTimeScale(1.0f);
    ecsPtr->SetState(RUNNING);



    time = 0.0f;
    maxTime = 1.5f; // adjust roll duration
    shakeTriggered = false;
    fadeTriggered = false;

    direction = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f));
    speed = 4.0f;

    octaves = 3;
    vertScale = 0.03f;
    horizScale = 0.015f;
    rotScale = 0.5f;

    vertSpeed = 0.8f;
    horizSpeed = 0.6f;
    rotSpeed = 0.5f;

    impactTimer = 0.0f;
    impactInterval = 6.0f;
    impactDuration = 0.05f;
    impactAmplitude = 0.1f;
    impactOffset = glm::vec3(0.0f);

    cameraShakeIntensity = 0.f;
    cameraShakeDuration = 0.f;
    cameraShakeElapsed = 0.f;
    isCameraShaking = false;

    endTimer = 0.f;
    fadeDelay = 1.5f;

    moveToNextScene = false;


    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;
    startPos = tf->LocalTransformation.position;
}

inline void MainMenuCameraScript::CameraShake(float intensity, float duration) {
    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    if (isCameraShaking) {
        tf->LocalTransformation.position = cameraShakeOriginalPos;
    }

    cameraShakeOriginalPos = tf->LocalTransformation.position;
    cameraShakeIntensity = intensity;
    cameraShakeDuration = duration;
    cameraShakeElapsed = 0.f;
    isCameraShaking = true;
}

inline void MainMenuCameraScript::Update() {
    auto* tf = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
    if (!tf) return;

    float dt = ecsPtr->m_GetDeltaTime();
    time += dt;

    // --- END SHAKE ---
    if (time >= maxTime) {
        if (!shakeTriggered) {
            CameraShake(0.1f, 0.15f); // shake once when rolling stops
            shakeTriggered = true;
        }

        // Start counting AFTER roll ends
        endTimer += dt;

        // Trigger fade after delay
        if (!fadeTriggered && endTimer >= fadeDelay) {
            fadeToBlackGOID = ecsPtr->GetEntityIDFromGUID(fadeToBlackGO);
            auto* fTB = ecsPtr->GetComponent<FadeTransition>(fadeToBlackGOID);
            if (fTB) {
                fTB->TransitionToBlack(2.0f);
            }
            fadeTriggered = true;
        }

        if (fadeTriggered && endTimer >= (fadeDelay + 2.5f) && !moveToNextScene) {
            moveToNextScene = true;
            Scenes->ClearAllScene();
            Scenes->LoadScene(next_Scene);
        }

        // --- UPDATE CAMERA SHAKE ---
        if (isCameraShaking) {
            cameraShakeElapsed += dt;
            float t = cameraShakeElapsed / cameraShakeDuration;
            if (t >= 1.f) {
                tf->LocalTransformation.position = cameraShakeOriginalPos;
                isCameraShaking = false;
                cameraShakeElapsed = 0.f;
            }
            else {
                float curveValue = 1.f - t;
                auto randF = [](float range) -> float {
                    return ((float)rand() / RAND_MAX) * 2.0f * range - range;
                    };
                glm::vec3 shakeOffset = glm::vec3(
                    randF(cameraShakeIntensity),
                    randF(cameraShakeIntensity),
                    0.f
                );
                tf->LocalTransformation.position = cameraShakeOriginalPos + shakeOffset;
            }
        }

        return; // stop rolling motion
    }

    // --- BASE MOTION ---
    glm::vec3 basePos = startPos + direction * speed * time;

    // --- PERLIN BUMPS ---
    float verticalOffset = PerlinOctave1D(time * vertSpeed, octaves) * vertScale;
    float horizontalOffset = PerlinOctave1D((time + 100.0f) * horizSpeed, octaves) * horizScale;
    glm::vec3 bumpOffset = glm::vec3(horizontalOffset, verticalOffset, 0.0f);

    // --- MICRO-IMPACTS (position only) ---
    impactTimer += dt;
    if (impactTimer >= impactInterval) {
        impactTimer = 0.0f;
        auto randF = [](float range) -> float {
            return ((float)rand() / RAND_MAX) * 2.0f * range - range;
            };
        impactOffset = glm::vec3(randF(impactAmplitude), randF(impactAmplitude), 0.0f);
    }

    glm::vec3 impactDecayOffset = impactOffset * std::exp(-impactTimer / impactDuration);

    tf->LocalTransformation.position = basePos + bumpOffset + impactDecayOffset;

    // --- ROTATIONAL JITTER ---
    float roll = PerlinOctave1D(time * rotSpeed, octaves) * rotScale;
    float pitch = PerlinOctave1D((time + 50.0f) * rotSpeed, octaves) * rotScale;
    float yaw = PerlinOctave1D((time + 100.0f) * rotSpeed, octaves) * rotScale;

    tf->LocalTransformation.rotation = glm::vec3(pitch, yaw, roll);


}
