#pragma once

#include "ScriptAdapter/TemplateSC.h"
#include <cmath>
#include <chrono>
#include <iostream>
#include "Utility/GUID.h"

class AudioScript : public TemplateSC {
public:
    utility::GUID GUID;

    float elapsedTime = 0.0f;
    float speed = 1.0f;  
    float radius = 5.0f;  

    std::chrono::high_resolution_clock::time_point prevTime{};

    void Start() override {
        prevTime = std::chrono::high_resolution_clock::now();

        auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity);
        auto* tr = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
        if (!ac || ac->audioFiles.empty() || !tr) return;

        for (auto& af : ac->audioFiles) {
            if (!af.playOnStart || af.audioGUID.Empty())
                continue;

            GUID = af.audioGUID;

            af.requestPlay = true;

            af.use3D = true;
            af.minDistance = 1.0f;
            af.maxDistance = 25.0f;

            const auto& pos = tr->WorldTransformation.position;
            std::cout << "[AudioScript] 3D audio started at ("
                << pos.x << ", " << pos.y << ", " << pos.z << ")\n";

            break;
        }
    }

    void Update() override {
        auto* tr = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
        if (!tr) return;

        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> delta = now - prevTime;
        prevTime = now;

        float dt = delta.count();
        elapsedTime += dt * speed;

        tr->WorldTransformation.position.x = std::cos(elapsedTime) * radius;
        tr->WorldTransformation.position.z = std::sin(elapsedTime) * radius;
    }

    REFLECTABLE(AudioScript, GUID, elapsedTime, speed, radius);
};
