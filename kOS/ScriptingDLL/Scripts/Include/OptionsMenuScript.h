#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"

// Written here, extern-declared in PlayerManagerScript.h and PauseMenuScript.h
inline bool  gOptionsMenuActive = false;
inline float gPlayerCameraSpeedX = 0.65f;
inline float gPlayerCameraSpeedY = 0.65f;



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

    // BGM Slider
    utility::GUID bgmVolumeSliderBarGUID;
    ecs::EntityID bgmVolumeSliderBarID = 0;
    utility::GUID bgmVolumeMinusButtonGUID;
    ecs::EntityID bgmVolumeMinusButtonID = 0;
    utility::GUID bgmVolumePlusButtonGUID;
    ecs::EntityID bgmVolumePlusButtonID = 0;
    static int bgmVolumeLevel;
    bool wasBgmMinusPressed = false;
    bool wasBgmPlusPressed = false;

    // SFX Slider
    utility::GUID sfxVolumeSliderBarGUID;
    ecs::EntityID sfxVolumeSliderBarID = 0;
    utility::GUID sfxVolumeMinusButtonGUID;
    ecs::EntityID sfxVolumeMinusButtonID = 0;
    utility::GUID sfxVolumePlusButtonGUID;
    ecs::EntityID sfxVolumePlusButtonID = 0;
    static int sfxVolumeLevel;
    bool wasSfxMinusPressed = false;
    bool wasSfxPlusPressed = false;

    // Mouse Sensitivity Slider
    utility::GUID mouseSensSliderBarGUID;
    ecs::EntityID mouseSensSliderBarID = 0;
    utility::GUID mouseSensMinusButtonGUID;
    ecs::EntityID mouseSensMinusButtonID = 0;
    utility::GUID mousesensPlusButtonGUID;
    ecs::EntityID mousesensPlusButtonID = 0;
    static int mouseSensLevel;
    bool wasMouseMinusPressed = false;
    bool wasMousePlusPressed = false;

    // ---------------------------------------------------------------
    // State
    // ---------------------------------------------------------------
    static OptionsMenuScript* instance;
    static bool isOptionsActive;

    // Volume level: 0 to 10  (each step = 10%)
    static int masterVolumeLevel;  // default: full volume

    // ---------------------------------------------------------------
    // Hidden position (off-screen, same pattern as PauseMenuScript)
    // ---------------------------------------------------------------
    glm::vec3 originalOptionsPosition;
    glm::vec3 hiddenPosition = glm::vec3(-10000.0f, -10000.0f, 0.0f);

private:
    // ---------------------------------------------------------------
    // Per-slider transform cache — one per slider so they don't
    // bleed into each other (the old version shared one cache).
    // ---------------------------------------------------------------
    struct SliderCache {
        glm::vec3 originalScale = glm::vec3(1.0f);
        glm::vec3 originalPosition = glm::vec3(0.0f);
        bool      initialized = false;
    };

    SliderCache masterSliderCache;
    SliderCache bgmSliderCache;
    SliderCache sfxSliderCache;
    SliderCache mouseSensSliderCache;

    // Button debounce
    bool wasMinusPressed = false;
    bool wasPlusPressed = false;

public:

    // ---------------------------------------------------------------
    void Start() override {
        instance = this;
        isOptionsActive = false;
        gOptionsMenuActive = false;

        // Resolve entity IDs from GUIDs
        optionsMenuCanvasID = ecsPtr->GetEntityIDFromGUID(optionsMenuCanvasGUID);
        pauseMenuManagerID = ecsPtr->GetEntityIDFromGUID(pauseMenuManagerGUID);
        masterVolumeSliderBarID = ecsPtr->GetEntityIDFromGUID(masterVolumeSliderBarGUID);
        masterVolumeMinusButtonID = ecsPtr->GetEntityIDFromGUID(masterVolumeMinusButtonGUID);
        masterVolumePlusButtonID = ecsPtr->GetEntityIDFromGUID(masterVolumePlusButtonGUID);

        bgmVolumeSliderBarID = ecsPtr->GetEntityIDFromGUID(bgmVolumeSliderBarGUID);
        bgmVolumeMinusButtonID = ecsPtr->GetEntityIDFromGUID(bgmVolumeMinusButtonGUID);
        bgmVolumePlusButtonID = ecsPtr->GetEntityIDFromGUID(bgmVolumePlusButtonGUID);

        sfxVolumeSliderBarID = ecsPtr->GetEntityIDFromGUID(sfxVolumeSliderBarGUID);
        sfxVolumeMinusButtonID = ecsPtr->GetEntityIDFromGUID(sfxVolumeMinusButtonGUID);
        sfxVolumePlusButtonID = ecsPtr->GetEntityIDFromGUID(sfxVolumePlusButtonGUID);

        if (!mouseSensSliderBarGUID.Empty())
            mouseSensSliderBarID = ecsPtr->GetEntityIDFromGUID(mouseSensSliderBarGUID);
        if (!mouseSensMinusButtonGUID.Empty())
            mouseSensMinusButtonID = ecsPtr->GetEntityIDFromGUID(mouseSensMinusButtonGUID);
        if (!mousesensPlusButtonGUID.Empty())
            mousesensPlusButtonID = ecsPtr->GetEntityIDFromGUID(mousesensPlusButtonGUID);

        // Cache original options canvas position
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            originalOptionsPosition = t->LocalTransformation.position;
        }

        // Cache ALL slider original transforms BEFORE any UpdateSliderBar call
        // modifies them — prevents later sliders reading an already-shrunk scale.
        CacheSlider(masterVolumeSliderBarID, masterSliderCache);
        CacheSlider(bgmVolumeSliderBarID, bgmSliderCache);
        CacheSlider(sfxVolumeSliderBarID, sfxSliderCache);
        CacheSlider(mouseSensSliderBarID, mouseSensSliderCache);

        // Start hidden
        SetOptionsMenuActive(false);

        // Reapply saved volumes to FMOD
        audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
        audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
        audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);

        // Redraw all sliders
        UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel, masterSliderCache);
        UpdateSliderBar(bgmVolumeSliderBarID, bgmVolumeLevel, bgmSliderCache);
        UpdateSliderBar(sfxVolumeSliderBarID, sfxVolumeLevel, sfxSliderCache);
        UpdateSliderBar(mouseSensSliderBarID, mouseSensLevel, mouseSensSliderCache);

        // Push default sensitivity into globals
        ApplyMouseSensitivityToPlayer();

        std::cout << "[OptionsMenuScript] Start() complete.\n";
        std::cout << "  masterVolumeLevel = " << masterVolumeLevel << "/10\n";
        std::cout << "  mouseSensLevel    = " << mouseSensLevel << "/10\n";
    }

    // ---------------------------------------------------------------
    void Update() override {
        if (!isOptionsActive) return;

        // ---- Master Volume ----
        if (masterVolumeMinusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(masterVolumeMinusButtonID)) {
                if (btn->isPressed && !wasMinusPressed)
                    DecreaseMasterVolume();
                wasMinusPressed = btn->isPressed;
            }
        }
        if (masterVolumePlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(masterVolumePlusButtonID)) {
                if (btn->isPressed && !wasPlusPressed)
                    IncreaseMasterVolume();
                wasPlusPressed = btn->isPressed;
            }
        }

        // ---- BGM ----
        if (bgmVolumeMinusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(bgmVolumeMinusButtonID)) {
                if (btn->isPressed && !wasBgmMinusPressed) DecreaseBGMVolume();
                wasBgmMinusPressed = btn->isPressed;
            }
        }
        if (bgmVolumePlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(bgmVolumePlusButtonID)) {
                if (btn->isPressed && !wasBgmPlusPressed) IncreaseBGMVolume();
                wasBgmPlusPressed = btn->isPressed;
            }
        }

        // ---- SFX ----
        if (sfxVolumeMinusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(sfxVolumeMinusButtonID)) {
                if (btn->isPressed && !wasSfxMinusPressed) DecreaseSFXVolume();
                wasSfxMinusPressed = btn->isPressed;
            }
        }
        if (sfxVolumePlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(sfxVolumePlusButtonID)) {
                if (btn->isPressed && !wasSfxPlusPressed) IncreaseSFXVolume();
                wasSfxPlusPressed = btn->isPressed;
            }
        }

        // ---- Mouse Sensitivity ----
        if (mouseSensMinusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(mouseSensMinusButtonID)) {
                if (btn->isPressed && !wasMouseMinusPressed) DecreaseMouseSensitivity();
                wasMouseMinusPressed = btn->isPressed;
            }
        }
        if (mousesensPlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(mousesensPlusButtonID)) {
                if (btn->isPressed && !wasMousePlusPressed) IncreaseMouseSensitivity();
                wasMousePlusPressed = btn->isPressed;
            }
        }
    }

    // ---------------------------------------------------------------
    // Called by UIButtonScript (action: Options / CloseOptions)
    // ---------------------------------------------------------------
    void OpenOptions() {
        isOptionsActive = true;
        gOptionsMenuActive = true;

        if (PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(false);

        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = originalOptionsPosition;
        }

        std::cout << "[OptionsMenuScript] Options opened, PauseMenu hidden.\n";
    }

    void CloseOptions() {
        isOptionsActive = false;
        gOptionsMenuActive = false;

        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = hiddenPosition;
        }

        if (PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(true);

        std::cout << "[OptionsMenuScript] Options closed, PauseMenu restored.\n";
    }

    // ---------------------------------------------------------------
    // Volume controls
    // ---------------------------------------------------------------
    void IncreaseMasterVolume() {
        if (masterVolumeLevel < 10) {
            masterVolumeLevel++;
            audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
            UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel, masterSliderCache);
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
        else {
            std::cout << "[OptionsMenuScript] Master Volume already at max (10/10)\n";
        }
    }

    void DecreaseMasterVolume() {
        if (masterVolumeLevel > 0) {
            masterVolumeLevel--;
            audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
            UpdateSliderBar(masterVolumeSliderBarID, masterVolumeLevel, masterSliderCache);
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
        else {
            std::cout << "[OptionsMenuScript] Master Volume already at min (0/10)\n";
        }
    }

    void IncreaseBGMVolume() {
        if (bgmVolumeLevel < 10) {
            bgmVolumeLevel++;
            audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
            UpdateSliderBar(bgmVolumeSliderBarID, bgmVolumeLevel, bgmSliderCache);
            std::cout << "[OptionsMenuScript] BGM Volume: " << bgmVolumeLevel << "/10\n";
        }
    }

    void DecreaseBGMVolume() {
        if (bgmVolumeLevel > 0) {
            bgmVolumeLevel--;
            audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
            UpdateSliderBar(bgmVolumeSliderBarID, bgmVolumeLevel, bgmSliderCache);
            std::cout << "[OptionsMenuScript] BGM Volume: " << bgmVolumeLevel << "/10\n";
        }
    }

    void IncreaseSFXVolume() {
        if (sfxVolumeLevel < 10) {
            sfxVolumeLevel++;
            audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);
            UpdateSliderBar(sfxVolumeSliderBarID, sfxVolumeLevel, sfxSliderCache);
            std::cout << "[OptionsMenuScript] SFX Volume: " << sfxVolumeLevel << "/10\n";
        }
    }

    void DecreaseSFXVolume() {
        if (sfxVolumeLevel > 0) {
            sfxVolumeLevel--;
            audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);
            UpdateSliderBar(sfxVolumeSliderBarID, sfxVolumeLevel, sfxSliderCache);
            std::cout << "[OptionsMenuScript] SFX Volume: " << sfxVolumeLevel << "/10\n";
        }
    }

    // ---------------------------------------------------------------
    // Mouse Sensitivity
    // ---------------------------------------------------------------
    void IncreaseMouseSensitivity() {
        if (mouseSensLevel < 10) {
            mouseSensLevel++;
            ApplyMouseSensitivityToPlayer();
            UpdateSliderBar(mouseSensSliderBarID, mouseSensLevel, mouseSensSliderCache);
            std::cout << "[OptionsMenuScript] Mouse Sensitivity: " << mouseSensLevel << "/10\n";
        }
    }

    void DecreaseMouseSensitivity() {
        if (mouseSensLevel > 1) {
            mouseSensLevel--;
            ApplyMouseSensitivityToPlayer();
            UpdateSliderBar(mouseSensSliderBarID, mouseSensLevel, mouseSensSliderCache);
            std::cout << "[OptionsMenuScript] Mouse Sensitivity: " << mouseSensLevel << "/10\n";
        }
    }

private:

    void ApplyMouseSensitivityToPlayer() {
        float speed = mouseSensLevel * 0.1f;
        gPlayerCameraSpeedX = speed;
        gPlayerCameraSpeedY = speed;
    }

    // Cache a slider's original transform before anything modifies it
    void CacheSlider(ecs::EntityID sliderBarID, SliderCache& cache) {
        if (sliderBarID == 0 || cache.initialized) return;
        if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(sliderBarID)) {
            cache.originalScale = tc->LocalTransformation.scale;
            cache.originalPosition = tc->LocalTransformation.position;
            cache.initialized = true;
        }
    }

    // ---------------------------------------------------------------
    // Crops + scales the slider bar sprite based on level (0-10).
    // Each slider now gets its own cache so they don't affect each other.
    // ---------------------------------------------------------------
    void UpdateSliderBar(ecs::EntityID sliderBarID, int level, SliderCache& cache) {
        if (sliderBarID == 0 || !cache.initialized) return;

        float percentage = (float)level / 10.0f;

        // UV crop: trim the right side of the sprite
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(sliderBarID)) {
            sc->useCustomUV = true;
            sc->uvMin = glm::vec2(0.0f, 0.0f);
            sc->uvMax = glm::vec2(percentage, 1.0f);
        }

        // Scale + reposition so the LEFT edge stays anchored
        if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(sliderBarID)) {
            float scaleReduction = cache.originalScale.x * (1.0f - percentage);

            tc->LocalTransformation.scale.x = cache.originalScale.x * percentage;
            tc->LocalTransformation.scale.y = cache.originalScale.y;
            tc->LocalTransformation.scale.z = cache.originalScale.z;

            tc->LocalTransformation.position.x = cache.originalPosition.x - (scaleReduction * 0.5f);
            tc->LocalTransformation.position.y = cache.originalPosition.y;
            tc->LocalTransformation.position.z = cache.originalPosition.z;
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
        bgmVolumeSliderBarGUID, bgmVolumeSliderBarID,
        bgmVolumeMinusButtonGUID, bgmVolumeMinusButtonID,
        bgmVolumePlusButtonGUID, bgmVolumePlusButtonID,
        bgmVolumeLevel,
        sfxVolumeSliderBarGUID, sfxVolumeSliderBarID,
        sfxVolumeMinusButtonGUID, sfxVolumeMinusButtonID,
        sfxVolumePlusButtonGUID, sfxVolumePlusButtonID,
        sfxVolumeLevel,
        mouseSensSliderBarGUID, mouseSensSliderBarID,
        mouseSensMinusButtonGUID, mouseSensMinusButtonID,
        mousesensPlusButtonGUID, mousesensPlusButtonID,
        mouseSensLevel,
        originalOptionsPosition, hiddenPosition);
};

inline OptionsMenuScript* OptionsMenuScript::instance = nullptr;
inline bool               OptionsMenuScript::isOptionsActive = false;
inline int                OptionsMenuScript::masterVolumeLevel = 10;
inline int                OptionsMenuScript::bgmVolumeLevel = 10;
inline int                OptionsMenuScript::sfxVolumeLevel = 10;
inline int                OptionsMenuScript::mouseSensLevel = 5;
