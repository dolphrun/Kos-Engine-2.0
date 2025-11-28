#pragma once
#include "ECS/Component/Component.h"
#include "ScriptAdapter/TemplateSC.h"
#include <iostream>
#include "ECS/Component/TextComponent.h" // assuming you have a TextComponent

class ScoreManagerScript : public TemplateSC {
public:
    // Score values
    int currentScore = 0;
    int mult = 1; // multiplier starts at 1
    static int bestScore;
    static int lastScore;

    // GUIDs for dynamic text objects
    utility::GUID scoreTextGUID;
    utility::GUID multTextGUID;

    // Entity IDs assigned at Start
    ecs::EntityID scoreTextID = 0;
    ecs::EntityID multTextID = 0;

    void Start() override {
        currentScore = 0;
        bestScore = 5000;
        mult = 1;

        // Convert GUIDs to ECS entity IDs
        scoreTextID = ecsPtr->GetEntityIDFromGUID(scoreTextGUID);
        multTextID = ecsPtr->GetEntityIDFromGUID(multTextGUID);

        // Initialize text
        UpdateScoreText();
        UpdateMultText();
    }

    void Update() override {
        // Optional: debug print
        // std::cout << "Score: " << currentScore << " | Multiplier: " << mult << "\n";
    }

    void AddScore(int scoreAdded) {
        if (mult <= 0) mult = 1;

        currentScore += scoreAdded * mult;
        mult += 1; // increase multiplier on each kill

        TallyScore();

        // Update UI immediately
        UpdateScoreText();
        UpdateMultText();
    }

    void ResetMultiplier() {
        mult = 1;
        UpdateMultText();
    }

    void TallyScore() {
        if (currentScore > bestScore) bestScore = currentScore;
        lastScore = currentScore;
    }

private:
    void UpdateScoreText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(scoreTextID)) {
            textComp->text = std::to_string(currentScore); // Update UI
        }
        std::cout << "Current Score: " << currentScore << std::endl; // Print to console
    }

    void UpdateMultText() {
        if (auto* textComp = ecsPtr->GetComponent<ecs::TextComponent>(multTextID)) {
            textComp->text = std::to_string(mult) + "x"; // Update UI
        }
        std::cout << "Multiplier: " << mult << "x" << std::endl; // Print to console
    }


public:
    REFLECTABLE(ScoreManagerScript, currentScore, mult, scoreTextGUID, multTextGUID);
};

inline int ScoreManagerScript::bestScore = 0;
inline int ScoreManagerScript::lastScore = 0;
