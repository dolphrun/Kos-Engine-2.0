#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include <iostream>

class ScoreManagerScript : public TemplateSC {
public:
    // Static game statistics - tracked globally
    static float timeTaken;
    static int enemiesKilled;
    static int elementsAbsorbed;
    static int abilitiesUsed;
    static int damageTaken;
    static bool isTimerActive;

    // Static variables for final display
    static float lastTimeTaken;
    static int lastEnemiesKilled;
    static int lastElementsAbsorbed;
    static int lastAbilitiesUsed;
    static int lastDamageTaken;

    // Initialize/reset all stats (call at game start)
    static void Initialize() {
        timeTaken = 0.0f;
        enemiesKilled = 0;
        elementsAbsorbed = 0;
        abilitiesUsed = 0;
        damageTaken = 0;
        isTimerActive = true;
        std::cout << "ScoreManager initialized" << std::endl;
    }

    // Update timer (call this every frame from your game manager or player script)
    static void UpdateTimer(float deltaTime) {
        if (isTimerActive) {
            timeTaken += deltaTime;
        }
    }

    // Called when an enemy is killed
    static void AddEnemyKill() {
        enemiesKilled++;
        std::cout << "Enemies Killed: " << enemiesKilled << std::endl;
    }

    // Called when player presses 'E' to absorb element
    static void AddElementAbsorbed() {
        elementsAbsorbed++;
        std::cout << "Elements Absorbed: " << elementsAbsorbed << std::endl;
    }

    // Called when player uses an ability (LSHIFT, LMB, RMB)
    static void AddAbilityUsed() {
        abilitiesUsed++;
        std::cout << "Abilities Used: " << abilitiesUsed << std::endl;
    }

    // Called when player takes damage
    static void AddDamageTaken(int damage) {
        damageTaken += damage;
        std::cout << "Total Damage Taken: " << damageTaken << std::endl;
    }

    // Stop the timer (call when game ends)
    static void StopTimer() {
        isTimerActive = false;
    }

    // Save current stats to "last" variables (call before showing any end screen)
    static void FinalizeStats() {
        StopTimer();
        lastTimeTaken = timeTaken;
        lastEnemiesKilled = enemiesKilled;
        lastElementsAbsorbed = elementsAbsorbed;
        lastAbilitiesUsed = abilitiesUsed;
        lastDamageTaken = damageTaken;

        std::cout << "=== FINAL STATS ===" << std::endl;
        std::cout << "Time Taken: " << lastTimeTaken << "s" << std::endl;
        std::cout << "Enemies Killed: " << lastEnemiesKilled << std::endl;
        std::cout << "Elements Absorbed: " << lastElementsAbsorbed << std::endl;
        std::cout << "Abilities Used: " << lastAbilitiesUsed << std::endl;
        std::cout << "Damage Taken: " << lastDamageTaken << std::endl;
    }

public:
    REFLECTABLE(ScoreManagerScript, timeTaken, enemiesKilled, elementsAbsorbed, abilitiesUsed, damageTaken, isTimerActive);
};

// Static definitions
inline float ScoreManagerScript::timeTaken = 0.0f;
inline int ScoreManagerScript::enemiesKilled = 0;
inline int ScoreManagerScript::elementsAbsorbed = 0;
inline int ScoreManagerScript::abilitiesUsed = 0;
inline int ScoreManagerScript::damageTaken = 0;
inline bool ScoreManagerScript::isTimerActive = true;

inline float ScoreManagerScript::lastTimeTaken = 0.0f;
inline int ScoreManagerScript::lastEnemiesKilled = 0;
inline int ScoreManagerScript::lastElementsAbsorbed = 0;
inline int ScoreManagerScript::lastAbilitiesUsed = 0;
inline int ScoreManagerScript::lastDamageTaken = 0;