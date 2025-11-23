#pragma once

#include "ScriptAdapter/TemplateSC.h"
#include <cmath> 
#include "Utility/GUID.h"

class AudioScript : public TemplateSC {
public:
    utility::GUID GUID;

    // manual timer for movement
    float elapsedTime = 0.0f;
    float speed = 1.0f;   // radians per second
    float radius = 5.0f;  // movement radius

    void Start() override {
        auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity);
        auto* tr = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
        if (!ac || ac->audioFiles.empty() || !tr) return;

        for (auto& af : ac->audioFiles) {
            if (af.playOnStart && !af.audioGUID.Empty()) {
                GUID = af.audioGUID;
                af.requestPlay = true;

                //af.use3D = true;
                af.minDistance = 1.0f;
                af.maxDistance = 25.0f;

                const auto& pos = tr->WorldTransformation.position;
                std::cout << "[AudioScript] 3D audio started at ("
                    << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
                break;
            }
        }
    }

    void Update() override {

        static auto prevTime = std::chrono::high_resolution_clock::now();
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = now - prevTime;
        prevTime = now;

        float dt = delta.count();
        elapsedTime += dt * speed;

        //TEST 3D PAN
        auto* tr = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
        if (!tr) return;

        tr->WorldTransformation.position.x = std::cos(elapsedTime) * radius;
        tr->WorldTransformation.position.z = std::sin(elapsedTime) * radius;
    }

    REFLECTABLE(AudioScript, GUID);
};
