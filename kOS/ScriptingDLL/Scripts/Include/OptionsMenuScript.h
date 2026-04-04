#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"
#include "PauseMenuScript.h"


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
    // Per-slider transform cache
    // Each slider stores its own original scale/position so that
    // UpdateXxxSlider() only touches its own bar.
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

    // Button debounce (same wasPressed pattern as UIButtonScript)
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

        mouseSensSliderBarID = ecsPtr->GetEntityIDFromGUID(mouseSensSliderBarGUID);
        mouseSensMinusButtonID = ecsPtr->GetEntityIDFromGUID(mouseSensMinusButtonGUID);
        mousesensPlusButtonID = ecsPtr->GetEntityIDFromGUID(mousesensPlusButtonGUID);

        // Cache original options canvas position
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            originalOptionsPosition = t->LocalTransformation.position;
        }

        // Start hidden
        SetOptionsMenuActive(false);

        // Reapply saved volumes to FMOD
        audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
        audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
        audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);

        // Redraw all sliders (each caches its own transform on first call)
        UpdateMasterSlider();
        UpdateBGMSlider();
        UpdateSFXSlider();
        UpdateMouseSlider();

        // Apply saved sensitivity to player (if the player is already alive)
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
                if (btn->isPressed && !wasMinusPressed) DecreaseMasterVolume();
                wasMinusPressed = btn->isPressed;
            }
        }
        if (masterVolumePlusButtonID != 0) {
            if (auto* btn = ecsPtr->GetComponent<ecs::ButtonComponent>(masterVolumePlusButtonID)) {
                if (btn->isPressed && !wasPlusPressed) IncreaseMasterVolume();
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

        // Hide pause menu via instance
        if (PauseMenuScript::instance)
            PauseMenuScript::instance->SetPauseMenuActive(false);

        // Show options menu
        if (auto* t = ecsPtr->GetComponent<ecs::TransformComponent>(optionsMenuCanvasID)) {
            t->LocalTransformation.position = originalOptionsPosition;
        }

        std::cout << "[OptionsMenuScript] Options opened, PauseMenu hidden.\n";
    }

    void CloseOptions() {
        isOptionsActive = false;
        gOptionsMenuActive = false;

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
    // Master Volume
    // ---------------------------------------------------------------
    void IncreaseMasterVolume() {
        if (masterVolumeLevel < 10) {
            masterVolumeLevel++;
            audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
            UpdateMasterSlider();
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
    }

    void DecreaseMasterVolume() {
        if (masterVolumeLevel > 0) {
            masterVolumeLevel--;
            audioManager->SetMasterVolume(masterVolumeLevel / 10.0f);
            UpdateMasterSlider();
            std::cout << "[OptionsMenuScript] Master Volume: " << masterVolumeLevel << "/10\n";
        }
    }

    // ---------------------------------------------------------------
    // BGM Volume
    // ---------------------------------------------------------------
    void IncreaseBGMVolume() {
        if (bgmVolumeLevel < 10) {
            bgmVolumeLevel++;
            audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
            UpdateBGMSlider();
            std::cout << "[OptionsMenuScript] BGM Volume: " << bgmVolumeLevel << "/10\n";
        }
    }

    void DecreaseBGMVolume() {
        if (bgmVolumeLevel > 0) {
            bgmVolumeLevel--;
            audioManager->SetMusicVolume(bgmVolumeLevel / 10.0f);
            UpdateBGMSlider();
            std::cout << "[OptionsMenuScript] BGM Volume: " << bgmVolumeLevel << "/10\n";
        }
    }

    // ---------------------------------------------------------------
    // SFX Volume
    // ---------------------------------------------------------------
    void IncreaseSFXVolume() {
        if (sfxVolumeLevel < 10) {
            sfxVolumeLevel++;
            audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);
            UpdateSFXSlider();
            std::cout << "[OptionsMenuScript] SFX Volume: " << sfxVolumeLevel << "/10\n";
        }
    }

    void DecreaseSFXVolume() {
        if (sfxVolumeLevel > 0) {
            sfxVolumeLevel--;
            audioManager->SetSFXVolume(sfxVolumeLevel / 10.0f);
            UpdateSFXSlider();
            std::cout << "[OptionsMenuScript] SFX Volume: " << sfxVolumeLevel << "/10\n";
        }
    }

    // ---------------------------------------------------------------
    // Mouse Sensitivity
    // Level 0 is allowed but effectively stops mouse movement entirely;
    // keep minimum at 1 if you prefer a safety floor.
    // ---------------------------------------------------------------
    void IncreaseMouseSensitivity() {
        if (mouseSensLevel < 10) {
            mouseSensLevel++;
            ApplyMouseSensitivityToPlayer();
            UpdateMouseSlider();
            std::cout << "[OptionsMenuScript] Mouse Sensitivity: " << mouseSensLevel << "/10\n";
        }
    }

    void DecreaseMouseSensitivity() {
        if (mouseSensLevel > 1) {     // floor at 1 so the player can always look around
            mouseSensLevel--;
            ApplyMouseSensitivityToPlayer();
            UpdateMouseSlider();
            std::cout << "[OptionsMenuScript] Mouse Sensitivity: " << mouseSensLevel << "/10\n";
        }
    }

private:

    // ---------------------------------------------------------------
    // Writes the current sensitivity level into PlayerManagerScript.
    // playerCameraSpeedX/Y are the multipliers used in PlayerCameraControls().
    // Level 5 == 0.5f, same as the default of 0.65f is approximated at level 6/7.
    // Adjust the base multiplier below to taste.
    // ---------------------------------------------------------------
    void ApplyMouseSensitivityToPlayer() {
        float speed = mouseSensLevel * 0.1f;   // 0.1 .. 1.0
        gPlayerCameraSpeedX = speed;
        gPlayerCameraSpeedY = speed;
    }

    // ---------------------------------------------------------------
    // Per-slider update helpers
    // Each one caches its own bar's original transform independently,
    // so cropping one bar never bleeds into the others.
    // ---------------------------------------------------------------
    void UpdateMasterSlider() {
        ApplySliderVisual(masterVolumeSliderBarID, masterVolumeLevel, masterSliderCache);
    }

    void UpdateBGMSlider() {
        ApplySliderVisual(bgmVolumeSliderBarID, bgmVolumeLevel, bgmSliderCache);
    }

    void UpdateSFXSlider() {
        ApplySliderVisual(sfxVolumeSliderBarID, sfxVolumeLevel, sfxSliderCache);
    }

    void UpdateMouseSlider() {
        ApplySliderVisual(mouseSensSliderBarID, mouseSensLevel, mouseSensSliderCache);
    }

    // ---------------------------------------------------------------
    // Core slider visual logic now receives its own SliderCache so
    // every slider tracks its own original transform independently.
    // ---------------------------------------------------------------
    void ApplySliderVisual(ecs::EntityID sliderBarID, int level, SliderCache& cache) {
        if (sliderBarID == 0) return;

        float percentage = (float)level / 10.0f;   // 0.0 -> 1.0

        // UV crop: trim the right side of the sprite
        if (auto* sc = ecsPtr->GetComponent<ecs::SpriteComponent>(sliderBarID)) {
            sc->useCustomUV = true;
            sc->uvMin = glm::vec2(0.0f, 0.0f);
            sc->uvMax = glm::vec2(percentage, 1.0f);
        }

        // Scale + reposition so the LEFT edge stays anchored
        if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(sliderBarID)) {
            // Cache this slider's original transform on first visit
            if (!cache.initialized) {
                cache.originalScale = tc->LocalTransformation.scale;
                cache.originalPosition = tc->LocalTransformation.position;
                cache.initialized = true;
            }

            float scaleReduction = cache.originalScale.x * (1.0f - percentage);

            tc->LocalTransformation.scale.x = cache.originalScale.x * percentage;
            tc->LocalTransformation.scale.y = cache.originalScale.y;
            tc->LocalTransformation.scale.z = cache.originalScale.z;

            // Shift left by half the lost width so the left edge doesn't move
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
inline int                OptionsMenuScript::mouseSensLevel = 5;   // mid-range default
