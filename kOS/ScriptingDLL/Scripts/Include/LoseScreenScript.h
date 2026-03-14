#pragma once

#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "ScoreManagerScript.h"
#include <iostream>
#include <iomanip>
#include <sstream>

class LoseScreenScript : public TemplateSC {
public:
    // Static flag to indicate if lose screen is active
    static bool isLoseScreenActive;

    utility::GUID loseScreenCanvasGUID;
    ecs::EntityID loseScreenCanvasID = 0;

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

    glm::vec3 originalCanvasPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

    bool hasShownLoseScreen = false;

    void Start() override {
        isLoseScreenActive = false;
        loseScreenCanvasID = ecsPtr->GetEntityIDFromGUID(loseScreenCanvasGUID);

        // Get all stat text entity IDs
        timeTakenValueID = ecsPtr->GetEntityIDFromGUID(timeTakenValueGUID);
        enemiesKilledValueID = ecsPtr->GetEntityIDFromGUID(enemiesKilledValueGUID);
        elementsAbsorbedValueID = ecsPtr->GetEntityIDFromGUID(elementsAbsorbedValueGUID);
        abilitiesUsedValueID = ecsPtr->GetEntityIDFromGUID(abilitiesUsedValueGUID);
        damageTakenValueID = ecsPtr->GetEntityIDFromGUID(damageTakenValueGUID);

        if (auto* t = ecsPtr->GetComponent<TransformComponent>(loseScreenCanvasID)) {
            originalCanvasPosition = t->LocalTransformation.position;
        }

        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        // Always hide on start
        SetLoseScreenActive(false);
    }

    void Update() override {
        // Nothing needed here
    }

    // Call this when the player dies
    void ShowLoseScreen() {
        if (hasShownLoseScreen)
            return; // prevents double activation

        hasShownLoseScreen = true;
        isLoseScreenActive = true; // BLOCK pause menu
        ecsPtr->SetTimeScale(0.0f);
        ecsPtr->SetState(WAIT);
        SetLoseScreenActive(true);
        Input->HideCursor(false);

        ScoreManagerScript::FinalizeStats();

        // Update all stat displays
        UpdateTimeTakenText();
        UpdateEnemiesKilledText();
        UpdateElementsAbsorbedText();
        UpdateAbilitiesUsedText();
        UpdateDamageTakenText();

        std::cout << "LOSE SCREEN SHOWN WITH STATS" << std::endl;
    }

    // Optional: hide the lose screen if needed
    void HideLoseScreen() {
        if (!hasShownLoseScreen)
            return;

        hasShownLoseScreen = false;
        isLoseScreenActive = false; // unblock pause menu
        ecsPtr->SetTimeScale(1.0f);
        ecsPtr->SetState(RUNNING);
        SetLoseScreenActive(false);
        Input->HideCursor(true);

        std::cout << "LOSE SCREEN HIDDEN" << std::endl;
    }

private:
    void SetLoseScreenActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<TransformComponent>(loseScreenCanvasID)) {
            t->LocalTransformation.position = active ? originalCanvasPosition : hiddenPosition;
        }
    }

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
    REFLECTABLE(LoseScreenScript, loseScreenCanvasGUID, hiddenPosition, originalCanvasPosition,
        timeTakenValueGUID, enemiesKilledValueGUID, elementsAbsorbedValueGUID,
        abilitiesUsedValueGUID, damageTakenValueGUID);
};

// Static definition
bool LoseScreenScript::isLoseScreenActive = false;