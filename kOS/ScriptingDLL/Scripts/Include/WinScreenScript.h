#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"
#include <iostream>
#include <iomanip>
#include <sstream>

class WinScreenScript : public TemplateSC {
public:

    static WinScreenScript* instance;
    // Static flag to indicate if win screen is active
    static bool isWinScreenActive;

    utility::GUID winScreenCanvasGUID;
    ecs::EntityID winScreenCanvasID = 0;

    // GUIDs for stat display text components
    utility::GUID timeTakenValueGUID;
    utility::GUID enemiesKilledValueGUID;
    utility::GUID elementsAbsorbedValueGUID;
    utility::GUID abilitiesUsedValueGUID;
    utility::GUID damageTakenValueGUID;

    // Entity IDs for stat displays
    ecs::EntityID timeTakenValueID = 0;
    ecs::EntityID enemiesKilledValueID = 0;
    ecs::EntityID elementsAbsorbedValueID = 0;
    ecs::EntityID abilitiesUsedValueID = 0;
    ecs::EntityID damageTakenValueID = 0;

    //// Video background
    //utility::GUID winVideoGUID;
    //ecs::EntityID winVideoID = 0;

    //// Optional audio
    //utility::GUID winAudioGUID;
    //bool hasPlayedAudio = false;

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    bool hasShownWinScreen = false;

    void Start() override {
        instance = this;
        isWinScreenActive = false;
        winScreenCanvasID = ecsPtr->GetEntityIDFromGUID(winScreenCanvasGUID);

        // Get all stat text entity IDs
        timeTakenValueID = ecsPtr->GetEntityIDFromGUID(timeTakenValueGUID);
        enemiesKilledValueID = ecsPtr->GetEntityIDFromGUID(enemiesKilledValueGUID);
        elementsAbsorbedValueID = ecsPtr->GetEntityIDFromGUID(elementsAbsorbedValueGUID);
        abilitiesUsedValueID = ecsPtr->GetEntityIDFromGUID(abilitiesUsedValueGUID);
        damageTakenValueID = ecsPtr->GetEntityIDFromGUID(damageTakenValueGUID);

        //// Get video player
        //if (!winVideoGUID.Empty()) {
        //    winVideoID = ecsPtr->GetEntityIDFromGUID(winVideoGUID);
        //}

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(winScreenCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }

        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        // Always hide on start
        SetWinScreenActive(false);
    }

    void Update() override {
        // Nothing needed here - video will loop automatically if loop=true
    }

    // Call this when the player wins
    void ShowWinScreen() {
        if (hasShownWinScreen)
            return; // prevents double activation

        hasShownWinScreen = true;
        isWinScreenActive = true;
        /*hasPlayedAudio = false;*/
        ecsPtr->SetTimeScale(0.0f);
        ecsPtr->SetState(WAIT);
        SetWinScreenActive(true);
        Input->HideCursor(false);

        ScoreManagerScript::FinalizeStats();

        // Update all stat displays
        UpdateTimeTakenText();
        UpdateEnemiesKilledText();
        UpdateElementsAbsorbedText();
        UpdateAbilitiesUsedText();
        UpdateDamageTakenText();

        //// Start video playback
        //if (winVideoID != 0) {
        //    auto* vc = ecsPtr->GetComponent<ecs::VideoComponent>(winVideoID);
        //    if (vc) {
        //        vc->loop = true;   // Enable looping
        //        vc->pause = false; // Start playing
        //        std::cout << "Win screen video started" << std::endl;
        //    }
        //}

        //// Play audio if available
        //if (!winAudioGUID.Empty() && !hasPlayedAudio) {
        //    if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
        //        for (auto& af : ac->audioFiles) {
        //            if (af.audioGUID == winAudioGUID && af.isSFX) {
        //                af.requestPlay = true;
        //                hasPlayedAudio = true;
        //                break;
        //            }
        //        }
        //    }
        //}

        std::cout << "WIN SCREEN SHOWN WITH STATS" << std::endl;
    }

    // Optional: hide the win screen if needed
    void HideWinScreen() {
        if (!hasShownWinScreen)
            return;

        hasShownWinScreen = false;
        isWinScreenActive = false;
        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        SetWinScreenActive(false);
        Input->HideCursor(true);

        //// Stop video
        //if (winVideoID != 0) {
        //    auto* vc = ecsPtr->GetComponent<ecs::VideoComponent>(winVideoID);
        //    if (vc) {
        //        vc->pause = true;
        //    }
        //}

        std::cout << "WIN SCREEN HIDDEN" << std::endl;
    }

    void SetWinScreenActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(winScreenCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

private:


    void UpdateTimeTakenText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(timeTakenValueID)) {
            // Format time as _m __s (e.g., "2m 34s")
            float totalSeconds = ScoreManagerScript::lastTimeTaken;
            int minutes = static_cast<int>(totalSeconds) / 60;
            int seconds = static_cast<int>(totalSeconds) % 60;

            std::stringstream ss;
            ss << minutes << "m " << std::setw(2) << std::setfill('0') << seconds << "s";

            textComp->text = ss.str();
        }
    }

    void UpdateEnemiesKilledText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(enemiesKilledValueID)) {
            textComp->text = std::to_string(ScoreManagerScript::lastEnemiesKilled);
        }
    }

    void UpdateElementsAbsorbedText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(elementsAbsorbedValueID)) {
            textComp->text = std::to_string(ScoreManagerScript::lastElementsAbsorbed);
        }
    }

    void UpdateAbilitiesUsedText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(abilitiesUsedValueID)) {
            textComp->text = std::to_string(ScoreManagerScript::lastAbilitiesUsed);
        }
    }

    void UpdateDamageTakenText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(damageTakenValueID)) {
            textComp->text = std::to_string(ScoreManagerScript::lastDamageTaken);
        }
    }

public:
    REFLECTABLE(WinScreenScript, winScreenCanvasGUID, hiddenPosition, originalCanvasPosition,
        timeTakenValueGUID, enemiesKilledValueGUID, elementsAbsorbedValueGUID,
        abilitiesUsedValueGUID, damageTakenValueGUID);
};

// Static definition
bool WinScreenScript::isWinScreenActive = false;
inline WinScreenScript* WinScreenScript::instance = nullptr;