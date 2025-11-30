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

        ecs::AudioFile* selected = nullptr;

        if (!GUID.Empty()) {
            for (auto& af : ac->audioFiles) {
                if (!af.playOnStart)
                    continue;

                if (af.sourceType == AudioSourceType::Core) {
                    if (af.audioGUID.Empty())
                        continue;
                    if (!(af.audioGUID == GUID))
                        continue;
                }
                else if (af.sourceType == AudioSourceType::Studio) {
                    if (af.studioEventPath.empty())
                        continue;
                    if (af.audioBankGUID.Empty() && GUID.Empty())
                        continue;
                    if (!af.audioBankGUID.Empty() && !(af.audioBankGUID == GUID))
                        continue;
                    if (af.audioBankGUID.Empty() && !GUID.Empty()) {
                        af.audioBankGUID = GUID;
                    }
                }

                selected = &af;
                break;
            }
        }

        if (!selected) {
            for (auto& af : ac->audioFiles) {
                if (!af.playOnStart)
                    continue;

                if (af.sourceType == AudioSourceType::Core) {
                    if (af.audioGUID.Empty())
                        continue;
                }
                else if (af.sourceType == AudioSourceType::Studio) {
                    if (af.studioEventPath.empty())
                        continue;
                    if (af.audioBankGUID.Empty() && GUID.Empty())
                        continue;
                    if (af.audioBankGUID.Empty() && !GUID.Empty()) {
                        af.audioBankGUID = GUID;
                    }
                }

                selected = &af;
                break;
            }
        }

        if (!selected)
            return;

        selected->requestPlay = true;
        selected->use3D = true;
        selected->minDistance = 1.0f;
        selected->maxDistance = 25.0f;

        const auto& pos = tr->WorldTransformation.position;
        std::cout << "[AudioScript] 3D "
            << (selected->sourceType == AudioSourceType::Studio ? "Studio" : "Core")
            << " audio started at ("
            << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
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
