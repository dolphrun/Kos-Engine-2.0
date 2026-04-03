#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"



class OptionsMenuScript : public TemplateSC {
public:

    // ---------------------------------------------------------------
    // Inspector-assigned GUIDs
    // ---------------------------------------------------------------

    // Options Menu Canvas position control (same hide/show trick as PauseMenuScript)
    utility::GUID optionsMenuCanvasGUID;
    ecs::EntityID optionsMenuCanvasID = 0;

    // The GameObject that has PauseMenuScript attached
    utility::GUID pauseMenuManagerGUID;
    ecs::EntityID pauseMenuManagerID = 0;

    // Slider Bar entity for Master Volume (the sprite that gets cropped)
    utility::GUID masterVolumeSliderBarGUID;
    ecs::EntityID masterVolumeSliderBarID = 0;

    // - and + button entities for Master Volume
    utility::GUID masterVolumeMinusButtonGUID;
    ecs::EntityID masterVolumeMinusButtonID = 0;

    utility::GUID masterVolumePlusButtonGUID;
    ecs::EntityID masterVolumePlusButtonID = 0;

    // ---------------------------------------------------------------
    // State
    // ---------------------------------------------------------------
    static OptionsMenuScript* instance;
    static bool isOptionsActive;

    // Volume level: 0 to 10  (each step = 10%)
    int masterVolumeLevel = 10;  // default: full volume

    // ---------------------------------------------------------------
    // Hidden position (off-screen, same pattern as PauseMenuScript)
    // ---------------------------------------------------------------
    glm::vec3 originalOptionsPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

private:
    // Slider bar transform cache (same as NEWHealthBarScript)
    glm::vec3 sliderOriginalScale = glm::vec3(1.0f);
    glm::vec3 sliderOriginalPosition = glm::vec3(0.0f);
    bool sliderInitialized = false;

    // Button debounce (same wasPressed pattern as UIButtonScript)
    bool wasMinusPressed = false;
    bool wasPlusPressed = false;

public:

    // ---------------------------------------------------------------
    void Start() override {
        instance = this;
        isOptionsActive = false;

        // Resolve entity IDs from GUIDs
        optionsMenuCanvasID = ecsPtr->GetEntityIDFromGUID(optionsMenuCanvasGUID);
        pauseMenuManagerID = ecsPtr->GetEntityIDFromGUID(pauseMenuManagerGUID);
        masterVolumeSliderBarID = ecsPtr->GetEntityIDFromGUID(masterVolumeSliderBarGUID);
        masterVolumeMinusButtonID = ecsPtr->GetEntityIDFromGUID(masterVolumeMinusButtonGUID);
        masterVolumePlusButtonID = ecsPtr->GetEntityIDFromGUID(masterVolumePlusButtonGUID);

        // Cache original options canvas position
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            originalOptionsPosition = t->LocalTransformation.position;
        }

        // Cache slider bar original transform
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(masterVolumeSliderBarID)) {
            sliderOriginalScale = t->LocalTransformation.scale;
            sliderOriginalPosition = t->LocalTransformation.position;
            sliderInitialized = true;
        }

        // Start hidden
        SetOptionsMenuActive(false);

        // Draw slider at default volume
        UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel);

        std::cout << "[OptionsMenuScript] Start() complete.\n";
        std::cout << "  masterVolumeLevel = " << masterVolumeLevel << "/10\n";
    }

    // ---------------------------------------------------------------
    void Update() override {
        if (!isOptionsActive) return;

        // ---- - Button click ----
        if (masterVolumeMinusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(masterVolumeMinusButtonID)) {
                if (btn->isPressed && !wasMinusPressed)
                    DecreaseMasterVolume();
                wasMinusPressed = btn->isPressed;
            }
        }

        // ---- + Button click ----
        if (masterVolumePlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(masterVolumePlusButtonID)) {
                if (btn->isPressed && !wasPlusPressed)
                    IncreaseMasterVolume();
                wasPlusPressed = btn->isPressed;
            }
        }
    }

    // ---------------------------------------------------------------
    // Called by UIButtonScript (action: Options)
    // ---------------------------------------------------------------
    void OpenOptions() {
        isOptionsActive = true;

        // Hide pause menu via instance
        if (PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(false);

        // Show options menu
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = originalOptionsPosition;
        }

        std::cout << "[OptionsMenuScript] Options opened, PauseMenu hidden.\n";
    }

    // Called by UIButtonScript (action: CloseOptions)
    void CloseOptions() {
        isOptionsActive = false;

        // Hide options menu
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = hiddenPosition;
        }

        // Restore pause menu via instance
        if (PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(true);

        std::cout << "[OptionsMenuScript] Options closed, PauseMenu restored.\n";
    }

    // ---------------------------------------------------------------
    // Called by UIButtonScript (VolumeUp / VolumeDown actions)
    // ---------------------------------------------------------------
    void IncreaseMasterVolume() {
        if (masterVolumeLevel < 10) {
            masterVolumeLevel++;
            UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel);
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
        else {
            std::cout << "[OptionsMenuScript] Master Volume already at max (10/10)\n";
        }
    }

    void DecreaseMasterVolume() {
        if (masterVolumeLevel > 0) {
            masterVolumeLevel--;
            UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel);
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
        else {
            std::cout << "[OptionsMenuScript] Master Volume already at min (0/10)\n";
        }
    }

private:

    // ---------------------------------------------------------------
    // Crops + scales the slider bar sprite based on level (0-10)
    // Mirrors the exact technique in NEWHealthBarScript
    // ---------------------------------------------------------------
    void UpdateSliderBar(ecs::EntityID sliderBarID, int level) {
        float percentage = (float)level / 10.0f;  // 0.0 -> 1.0

        // UV crop: trim the right side of the sprite
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(sliderBarID)) {
            sc->useCustomUV = true;
            sc->uvMin = glm::vec2(0.0f, 0.0f);
            sc->uvMax = glm::vec2(percentage, 1.0f);
        }

        // Scale + reposition so the LEFT edge stays anchored
        if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(sliderBarID)) {
            if (!sliderInitialized) {
                sliderOriginalScale = tc->LocalTransformation.scale;
                sliderOriginalPosition = tc->LocalTransformation.position;
                sliderInitialized = true;
            }

            float scaleFactor = percentage;
            float scaleReduction = sliderOriginalScale.x * (1.0f - scaleFactor);

            tc->LocalTransformation.scale.x = sliderOriginalScale.x * scaleFactor;
            tc->LocalTransformation.scale.y = sliderOriginalScale.y;
            tc->LocalTransformation.scale.z = sliderOriginalScale.z;

            // Shift left by half the lost width so the left edge doesn't move
            tc->LocalTransformation.position.x = sliderOriginalPosition.x - (scaleReduction * 0.5f);
            tc->LocalTransformation.position.y = sliderOriginalPosition.y;
            tc->LocalTransformation.position.z = sliderOriginalPosition.z;
        }
    }

    void SetOptionsMenuActive(bool active) {
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = active ? originalOptionsPosition : hiddenPosition;
        }
    }

public:
    REFLECTABLE(OptionsMenuScript,
        optionsMenuCanvasGUID, optionsMenuCanvasID,
        pauseMenuManagerGUID, pauseMenuManagerID,
        masterVolumeSliderBarGUID, masterVolumeSliderBarID,
        masterVolumeMinusButtonGUID, masterVolumeMinusButtonID,
        masterVolumePlusButtonGUID, masterVolumePlusButtonID,
        masterVolumeLevel,
        originalOptionsPosition, hiddenPosition,
        sliderOriginalScale, sliderOriginalPosition, sliderInitialized,
        wasMinusPressed, wasPlusPressed);
};

inline OptionsMenuScript* OptionsMenuScript::instance = nullptr;
inline bool OptionsMenuScript::isOptionsActive = false;
