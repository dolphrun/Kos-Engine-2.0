#pragma once
#include "TemplateSC.h"
#include "PauseMenuScript.h"
#include "LoseScreenScript.h"
#include "WinScreenScript.h"

// --- FORWARD DECLARATIONS ---
// Tell the compiler these classes exist first, preventing circular dependency crashes
class BulletLogic;
class FireLMB;
class AcidLMB;
class LightningLMB;
class FirePowerupManagerScript;
class AcidPowerupManagerScript;
class LightningPowerupManagerScript;
class PowerupManagerScript;
class GroundCheckScript;

class PlayerManagerScript : public TemplateSC {
public:

	R_AnimController* playerController = nullptr;
	AnimatorComponent* animComp = nullptr;
	AnimState currAnimationState;

	// POWERUP PREFS
	enum Powerup {
		NONE = 0,
		FIRE = 1,
		ACID = 2,
		LIGHTNING = 3,
	};

	float maxMana = 30.f;
	float currMana = 0.f;

	// BASE PREFS
	int baseMaxBullets = 6;
	int baseCurrBullets = 6;
	float baseShootCooldown = 0.5f;
	float baseCurrShootCooldown = 0.f;

	// FIRE PREFS
	float fireMeleeCooldown = 1.f; // In seconds
	float fireCurrMeleeCooldown = 0.f;

	float fireAbilityCost = 30.f;

	float fireMovementCost = 5.f;
	float fireMovementCooldown = 1.f;
	float fireCurrMovementCooldown = 0.f;

	// LIGHTNING PREFS
	int lightningMaxBullets = 30;
	int lightningCurrBullets = 30;
	float lightningShootCooldown = 0.15f;
	float lightningCurrShootCooldown = 0.f;

	float lightningAbilityCost = 30.f;

	float lightningMovementCost = 5.f;
	float lightningMovementCooldown = 1.f;
	float lightningCurrMovementCooldown = 0.f;

	// ACID PREFS
	int acidMaxBullets = 3;
	int acidCurrBullets = 3;
	float acidShootCooldown = 0.75f;
	float acidCurrShootCooldown = 0.f;

	float acidAbilityCost = 20.f;

	float acidMovementCost = 5.f;
	float acidMovementCooldown = 2.f;
	float acidCurrMovementCooldown = 0.f;

	// PLAYER PREFS
	int maxPlayerHitPoints = 6;
	int currPlayerHitPoints;
	bool isDead = false;

	float maxPlayerMovSpeed = 10.0f;
	float currPlayerMovSpeed;

	float maxPlayerJumpForce = 8.0f;
	float currPlayerJumpForce;

	float playerCameraSpeedX = 0.65f;
	float playerCameraSpeedY = 0.65f;

	float interactPowerupRange = 10.f;

	float playerSprintMultiplier = 1.5f;
	float playerCrouchMultiplier = 0.5f;
	float playerSlideMultiplier = 5.0f;

	float playerCrouchTransitionSpeed = 2.f;

	float playerSprintFOVSpeed = 3.f;
	float playerSprintFOV = 100.f;
	float playerNormalFOV = 85.f;

	float playerVelocityBeforeSlide = 8.0f;

	float playerGunModelSwaySpeed = 15.f;
	float playerGunModelWalkBobbingSpeed = 7.0f;
	float playerGunModelSprintBobbingSpeed = 50.f;
	float playerGunModelBobbingIntensity = 0.045f;

	Powerup playerPowerupHeld = Powerup::NONE;

	// RELEVANT OBJECTS
	utility::GUID playerCameraObject;
	utility::GUID playerGunCameraObject;
	utility::GUID playerProjectilePointObject;
	utility::GUID playerGunModelPointObject;
	utility::GUID playerArmModelObject;
	utility::GUID playerGroundCheckObject;
	utility::GUID pauseMenuManagerObject;
	utility::GUID healthUIObject;
	utility::GUID loseScreenCanvasObject;
	utility::GUID winScreenCanvasObject;

	ecs::EntityID playerCameraObjectID;
	ecs::EntityID playerGunCameraObjectID;
	ecs::EntityID playerProjectilePointObjectID;
	ecs::EntityID playerGunModelPointObjectID;
	ecs::EntityID playerArmModelObjectID;
	ecs::EntityID playerGroundCheckObjectID;
	ecs::EntityID pauseMenuManagerID;
	ecs::EntityID healthUIObjectID;
	ecs::EntityID loseScreenCanvasID;
	ecs::EntityID winScreenCanvasID;
	ecs::EntityID fireDashID;

	utility::GUID bulletPrefab;
	utility::GUID fireLMBPrefab;
	utility::GUID acidLMBPrefab;
	utility::GUID lightningLMBPrefab;

	utility::GUID firePrefab;
	utility::GUID acidPrefab;
	utility::GUID lightningPrefab;

	utility::GUID fireDashPrefab;
	utility::GUID lightningDashPrefab;
	utility::GUID acidShieldPrefab;
	utility::GUID airBlastPrefab;

	// BACKEND PLAYER DETAILS
	float playerRotationX = 0.f, playerRotationY = 0.f;

	bool isMoving = false;
	bool playerIsWalking = false;
	bool playerIsSprinting = false;
	bool playerIsCrouching = false;
	bool playerIsSliding = false;

	bool isDashing = false;
	float dashDuration = 0.25f; // How long the player can't be controlled during dash?
	float currentDashTimer = 0.0f;

	float originalPlayerCrouchCameraPosY;
	float playerCrouchCameraPosY;

	float gunRotationY = 90.f;
	float gunRotationX = 0.f;

	float bobbingTimer = 0.f;

	//Shooting pref
	
	bool isReloading = false;
	float reloadDuration = 1.25f;
	float currentReloadTimer = 0.0f;

	bool autoReload = true;

	//Acid Shield Pref
	float acidShieldCost = 10.f; 
	float acidShieldCooldown = 6.f; 
	float acidCurrShieldCooldown = 0.f;

	// LIGHTNING LMB - TIMESLOW PREFS
	float lightningTimeslowDuration = 2.0f;
	float lightningTimeslowCost = 10.0f;
	float lightningTimeslowCooldown = 4.0f;
	float lightningCurrTimeslowCooldown = 0.0f;
	float lightningCurrTimeslowTimer = 0.0f;
	bool  isTimeslowActive = false;

	float groundAcceleration = 15.f;
	float airAcceleration = 25.f;
	float groundFriction = 8.f;
	float airControl = 0.3f;
	float maxGroundSpeed = 20.f;
	float maxAirSpeed = 20.f;
	float jumpForce = 12.f;


	float cameraTiltMaxAngle = 3.f;	// Max roll degrees left/right
	float cameraTiltSpeed = 8.f;		// How fast it lerps to target
	float cameraTiltReturnSpeed = 10.f; // How fast it returns to 0
	float cameraCurrTiltZ = 0.f;		//current Z roll

	float cameraShakeIntensity = 0.f;
	float cameraShakeDuration = 0.f;
	float cameraShakeTimer = 0.f;
	float cameraShakeDelay = 0.f;
	float cameraShakeElapsed = 0.f;
	float cameraTiltSmoothedInput = 0.f;
	bool  isCameraShaking = false;
	glm::vec3 cameraShakeOriginalPos = glm::vec3(0.f);
	glm::vec3 cameraShakeOffset = glm::vec3(0.f);


	inline int GetMaxBulletsForCurrentWeapon() const {
		switch (playerPowerupHeld) {
		case Powerup::LIGHTNING: return lightningMaxBullets;
		case Powerup::ACID:      return acidMaxBullets;
		default:                return baseMaxBullets; 
		}
	}

	inline int& GetCurrBulletsForCurrentWeapon() {
		switch (playerPowerupHeld) {
		case Powerup::LIGHTNING: return lightningCurrBullets;
		case Powerup::ACID:      return acidCurrBullets;
		default:                return baseCurrBullets;
		}
	}

	inline float GetShootCooldownForCurrentWeapon() const {
		switch (playerPowerupHeld) {
		case Powerup::LIGHTNING: return lightningShootCooldown;
		case Powerup::ACID:      return acidShootCooldown;
		default:                return baseShootCooldown;
		}
	}

	inline float& GetCurrShootCooldownForCurrentWeapon() {
		switch (playerPowerupHeld) {
		case Powerup::LIGHTNING: return lightningCurrShootCooldown;
		case Powerup::ACID:      return acidCurrShootCooldown;
		default:                return baseCurrShootCooldown;
		}
	}

	inline void StartReload() {
		if (isReloading) return;

		int& curr = GetCurrBulletsForCurrentWeapon();
		const int maxBullet = GetMaxBulletsForCurrentWeapon();
		if (curr >= maxBullet) return;

		isReloading = true;
		currentReloadTimer = reloadDuration;

		//Reload anim
		if (animComp)
		{
			if (animComp->m_currentStateID)
				playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("reloading", animComp, playerController);
		}
		// Reload sfx
		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

			for (auto& af : ac->audioFiles) {
				if (af.audioGUID == gunReloadSfxGUID && af.isSFX) {
					af.requestPlay = true;
					break;
				}
			}
		}

	}

	inline void FinishReload() {
		int& curr = GetCurrBulletsForCurrentWeapon();
		curr = GetMaxBulletsForCurrentWeapon();

		isReloading = false;
		currentReloadTimer = 0.0f;
	}

	// SFX
	utility::GUID gunSfxGUID_1;
	utility::GUID gunReloadSfxGUID;
	utility::GUID fireSlashSfxGUID;
	utility::GUID fireDashSfxGUID;

	utility::GUID lightningDashSfxGUID;
	utility::GUID lightningGunSfxGUID;

	//Dash VFX Timer
	float fireDashVfxTimer = 0.0f;
	float fireDashVfxDuration = 30.0f;





	// --- FUNCTION DECLARATIONS ONLY ---
	// Implementations are moved to the bottom of the file
	void Start() override;
	void Update() override;
	void FixedUpdate() override;

	void PlayerMovementControls();
	void PlayerCameraControls();
	void PlayerCombatControls();
	void CameraShake(float intensity, float duration);

	bool GroundCheck();
	//void TakeDamage(int amount); // Commented out in original
	//void Die(); // Commented out in original
	void UpdateHealthUI();

	// HELPER FUNCTIONS
	glm::vec3 GetPlayerCameraFrontDirection();
	glm::vec3 GetPlayerCameraRightDirection();
	glm::vec3 GetPlayerCameraUpDirection();
	glm::vec3 GetPlayerFrontDirection();
	glm::vec3 GetPlayerRightDirection();

	REFLECTABLE(PlayerManagerScript, playerCameraObject, playerGunCameraObject, playerProjectilePointObject, playerGunModelPointObject, playerArmModelObject, playerGroundCheckObject,
		bulletPrefab, fireLMBPrefab, acidLMBPrefab, lightningLMBPrefab, firePrefab, acidPrefab, lightningPrefab, fireDashPrefab, lightningDashPrefab, acidShieldPrefab, airBlastPrefab,
		gunSfxGUID_1,gunReloadSfxGUID, fireSlashSfxGUID, fireDashSfxGUID, lightningDashSfxGUID, lightningGunSfxGUID, pauseMenuManagerObject, healthUIObject, loseScreenCanvasObject, winScreenCanvasObject);
};

// --- LATE INCLUDES & IMPLEMENTATION ---
// We include the dependencies HERE, after the class is fully defined.
#include "BulletLogic.h"
#include "FireLMB.h"
#include "AcidLMB.h"
#include "LightningLMB.h"
#include "PowerupManagerScript.h"
#include "GroundCheckScript.h"

#include "FirePowerupManagerScript.h"
#include "LightningPowerupManagerScript.h"
#include "AcidPowerupManagerScript.h"
#include "FireLightningPowerupManagerScript.h"
#include "FireAcidPowerupManagerScript.h"
#include "LightningAcidPowerupManagerScript.h"

inline void PlayerManagerScript::Start() {
	playerCameraObjectID = ecsPtr->GetEntityIDFromGUID(playerCameraObject);
	playerGunCameraObjectID = ecsPtr->GetEntityIDFromGUID(playerGunCameraObject);
	playerProjectilePointObjectID = ecsPtr->GetEntityIDFromGUID(playerProjectilePointObject);
	playerGunModelPointObjectID = ecsPtr->GetEntityIDFromGUID(playerGunModelPointObject);
	playerArmModelObjectID = ecsPtr->GetEntityIDFromGUID(playerArmModelObject);
	playerGroundCheckObjectID = ecsPtr->GetEntityIDFromGUID(playerGroundCheckObject);

	currPlayerHitPoints = maxPlayerHitPoints;
	currPlayerMovSpeed = maxPlayerMovSpeed;
	currPlayerJumpForce = maxPlayerJumpForce;

	playerCrouchCameraPosY = originalPlayerCrouchCameraPosY = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y;
	playerCrouchCameraPosY -= 0.85f;

	Input->HideCursor(true);
	if (pauseMenuManagerObject != utility::GUID{}) {
		pauseMenuManagerID = ecsPtr->GetEntityIDFromGUID(pauseMenuManagerObject);
		//std::cout << "PlayerManager connected to PauseMenuManager!\n";
	}
	healthUIObjectID = ecsPtr->GetEntityIDFromGUID(healthUIObject);
	loseScreenCanvasID = ecsPtr->GetEntityIDFromGUID(loseScreenCanvasObject);
	winScreenCanvasID = ecsPtr->GetEntityIDFromGUID(winScreenCanvasObject);

	std::vector<EntityID> armChild = ecsPtr->GetChild(playerArmModelObjectID).value();
	if (animComp = ecsPtr->GetComponent<ecs::AnimatorComponent>(armChild[0]))
	{
		playerController = resource->GetResource<R_AnimController>(animComp->controllerGUID).get();
		if (playerController)
		{
			// COMMENTED OUT FOR ANIM
			/*currAnimationState = *playerController->m_EnterState;
			anim->m_currentState = &currAnimationState;
			static_cast<AnimState*>(anim->m_currentState)->SetTrigger("ForcedEntry");*/
			animComp->m_currentStateID = playerController->m_EnterState->id;
			if (auto* currAnimState = playerController->RetrieveStateByID(animComp->m_currentStateID))
				currAnimState->Trigger("ForcedEntry", animComp, playerController);
		}
	};
}

inline void PlayerManagerScript::Update() {

	if (Input->IsKeyTriggered(keys::L)) {
		//std::cout << "L RELEASED\n";
		Scenes->ReloadScene();

		if (auto* pauseManager = ecsPtr->GetComponent<PauseMenuScript>(pauseMenuManagerID)) {
			if (pauseManager->isPaused) {
				pauseManager->TogglePause();
			}
		}

	}

	if (Input->IsKeyTriggered(keys::ESC)) {
		if (auto* pauseManager = ecsPtr->GetComponent<PauseMenuScript>(pauseMenuManagerID)) {
			//std::cout << "PAUSE PAUSE PAUSE\n";
			pauseManager->TogglePause();
		}
	}

	if (currPlayerHitPoints <= 0 && !LoseScreenScript::isLoseScreenActive) {
		if (auto* loseScreen = ecsPtr->GetComponent<LoseScreenScript>(loseScreenCanvasID)) {
			loseScreen->ShowLoseScreen();
			return;
		}
	}

	if (PauseMenuScript::isPaused ||
		WinScreenScript::isWinScreenActive ||
		LoseScreenScript::isLoseScreenActive)
	{
		return; // Skip ALL player input
	}

	{
		float& cd = GetCurrShootCooldownForCurrentWeapon();
		if (cd > 0.0f) cd -= ecsPtr->m_GetDeltaTime();
		if (cd < 0.0f) cd = 0.0f;
	}

	// R to manual relod
	if (Input->IsKeyTriggered(keys::R)) {
		StartReload();
	}

	if (isReloading) {
		currentReloadTimer -= ecsPtr->m_GetDeltaTime();
		if (currentReloadTimer <= 0.0f) {
			FinishReload();
		}
	}

	if (fireCurrMovementCooldown >= 0.f) {
		fireCurrMovementCooldown -= ecsPtr->m_GetDeltaTime();
	}

	if (acidCurrMovementCooldown >= 0.f) {
		acidCurrMovementCooldown -= ecsPtr->m_GetDeltaTime();
	}

	if (acidCurrShieldCooldown > 0.f) { 
		acidCurrShieldCooldown -= ecsPtr->m_GetDeltaTime();    
		if (acidCurrShieldCooldown < 0.f){
			acidCurrShieldCooldown = 0.f;
		}	
	}

	if (lightningCurrMovementCooldown >= 0.f) {
		lightningCurrMovementCooldown -= ecsPtr->m_GetDeltaTime();
	}
	
	// LMB Ability Countdowns
	if (fireCurrMeleeCooldown > 0.0f)
	{
		fireCurrMeleeCooldown -= ecsPtr->m_GetDeltaTime();
		if (fireCurrMeleeCooldown < 0.0f)
			fireCurrMeleeCooldown = 0.0f;
	}

	// Dashing cooldown
	if (isDashing) {
		currentDashTimer -= ecsPtr->m_GetDeltaTime();
		if (currentDashTimer <= 0.0f) {
			isDashing = false;
		}
	}

	if (!isDashing) {
		PlayerMovementControls(); // Look at me
	}

	if (Input->IsKeyTriggered(keys::T)) {
		CameraShake(0.3f, 0.5f);   // flat-ass light shake
	}
	if (Input->IsKeyTriggered(keys::Y)) {
		CameraShake(0.6f, 0.4f);   // mid medium shake
	}
	if (Input->IsKeyTriggered(keys::U)) {
		CameraShake(1.2f, 0.6f);   // nicki minaj ass heavy shake
	}


	// PlayerMovementControls(); I removed dis to disable movement while dashing, if anyone wants there to be more control during a dash need to look here
	PlayerCameraControls();
	PlayerCombatControls();

	if (Input->IsKeyPressed(keys::SPACE)) {
		//std::cout << "[DEBUG] Space pressed - Grounded: " << GroundCheck() << std::endl;
	}

	// Timeslow countdown 
	if (isTimeslowActive) {
		lightningCurrTimeslowTimer -= ecsPtr->m_GetDeltaTime();
		if (lightningCurrTimeslowTimer <= 0.0f) {
			isTimeslowActive = false;
			lightningCurrTimeslowTimer = 0.0f;
			ecsPtr->SetTimeScale(1.0f);
		}
	}

	if (lightningCurrTimeslowCooldown > 0.f) {
		lightningCurrTimeslowCooldown -= ecsPtr->m_GetDeltaTime();
		if (lightningCurrTimeslowCooldown < 0.f)
			lightningCurrTimeslowCooldown = 0.f;
	}


}

inline void PlayerManagerScript::FixedUpdate() {

}

//inline void PlayerManagerScript::PlayerMovementControls() {
//	auto* playerRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity);
//	if (!playerRigidbody) return;
//
//	// Calculate current horizontal velocity
//	glm::vec3 currentVelocity = playerRigidbody->velocity;
//	glm::vec3 horizontalVelocity = glm::vec3(currentVelocity.x, 0.f, currentVelocity.z);
//	float currentSpeed = glm::length(horizontalVelocity);
//
//	// Determine target speed based on movement state
//	float targetSpeed = 0.f;
//	isMoving = (std::abs(Input->GetHorizontal()) > 0.1f || std::abs(Input->GetVertical()) > 0.1f);
//
//	// SPRINTING
//	if (Input->IsKeyPressed(keys::LeftShift) && Input->GetVertical() > 0.f && GroundCheck() && !playerIsCrouching) {
//		playerIsSprinting = true;
//		targetSpeed = maxPlayerMovSpeed * playerSprintMultiplier; // 15 m/s
//	}
//
//	// CROUCHING (not sliding)
//	else if (playerIsCrouching && !playerIsSliding) {
//		targetSpeed = maxPlayerMovSpeed * playerCrouchMultiplier; // 5 m/s
//	}
//
//	// WALKING
//	else if (isMoving) {
//		playerIsWalking = true;
//		targetSpeed = maxPlayerMovSpeed; // 10 m/s
//	}
//
//	// CROUCH/SLIDE HANDLING
//	if (Input->IsKeyPressed(keys::LeftControl) && !playerIsSliding && GroundCheck()) {
//		if (!playerIsCrouching) {
//			playerIsCrouching = true;
//
//			// Check if we should slide
//			if (currentSpeed >= playerVelocityBeforeSlide) {
//				playerIsSliding = true;
//
//				// Apply slide impulse in current movement direction
//				glm::vec3 slideDirection = glm::length(horizontalVelocity) > 0.1f ?
//					glm::normalize(horizontalVelocity) :
//					GetPlayerFrontDirection();
//
//				glm::vec3 slideForce = slideDirection * playerSlideMultiplier;
//				physicsPtr->AddForce(playerRigidbody->actor, slideForce, ForceMode::Impulse);
//			}
//		}
//	}
//
//	else if (Input->IsKeyReleased(keys::LeftControl)) {
//		playerIsCrouching = false;
//		playerIsSliding = false;
//	}
//
//	// MOVEMENT (don't apply if sliding - let physics handle it)
//	if (!playerIsSliding && isMoving) {
//		glm::vec3 inputDirection = GetPlayerFrontDirection() * Input->GetVertical() +
//			GetPlayerRightDirection() * Input->GetHorizontal();
//
//		if (glm::length2(inputDirection) > glm::epsilon<float>()) {
//			inputDirection = glm::normalize(inputDirection);
//
//			// Calculate desired velocity
//			glm::vec3 desiredVelocity = inputDirection * targetSpeed;
//
//			// Calculate the force needed (proportional to velocity difference)
//			glm::vec3 velocityError = desiredVelocity - horizontalVelocity;
//
//			// Apply acceleration force (this value controls responsiveness)
//			float groundAcceleration = 25.f; // Units/s² - adjust for feel
//			glm::vec3 moveForce = velocityError * groundAcceleration;
//
//			physicsPtr->AddForce(playerRigidbody->actor, moveForce, ForceMode::Acceleration);
//		}
//	}
//
//	// STOPPING - apply friction when no input and on ground
//	else if (!playerIsSliding && GroundCheck() && !isMoving && currentSpeed > 0.1f) {
//		float stopDeceleration = 20.f; // How fast to stop
//		glm::vec3 stopForce = -glm::normalize(horizontalVelocity) * stopDeceleration;
//		physicsPtr->AddForce(playerRigidbody->actor, stopForce, ForceMode::Acceleration);
//	}
//
//	// SLIDING FRICTION - slow down while sliding
//	if (playerIsSliding) {
//		float slideFriction = 10.f; // How fast slide decays
//		if (currentSpeed > maxPlayerMovSpeed * playerCrouchMultiplier) {
//			glm::vec3 frictionForce = -glm::normalize(horizontalVelocity) * slideFriction;
//			physicsPtr->AddForce(playerRigidbody->actor, frictionForce, ForceMode::Acceleration);
//		}
//		else {
//			// Stop sliding when we slow down enough
//			playerIsSliding = false;
//		}
//	}
//
//	// JUMPING
//	if (Input->IsKeyTriggered(keys::SPACE) && GroundCheck()) {
//		glm::vec3 jumpVelocity(0.f, currPlayerJumpForce, 0.f);
//		physicsPtr->AddForce(playerRigidbody->actor, jumpVelocity, ForceMode::VelocityChange);
//	}
//
//	// Reset flags
//	playerIsWalking = false;
//	playerIsSprinting = false;
//}

inline void PlayerManagerScript::PlayerMovementControls()
{
	float forward = Input->GetVertical();
	float right = Input->GetHorizontal();

	isMoving = (std::abs(forward) > 0.1f || std::abs(right) > 0.1f);


	auto* playerRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity);
	if (!playerRigidbody) return;

	float dt = ecsPtr->m_GetDeltaTime();

	glm::vec3 tempVelocity = playerRigidbody->velocity;

	bool grounded = GroundCheck();

	// ----- INPUT -----

	glm::vec3 wishDir =
		GetPlayerFrontDirection() * forward +
		GetPlayerRightDirection() * right;

	if (glm::length2(wishDir) > 0.0001f)
		wishDir = glm::normalize(wishDir);

	//grounded = true;

	// ==============================
	// GROUND MOVEMENT
	// ==============================
	if (grounded)
	{
		// old friction code
		//float speed = glm::length(glm::vec2(tempVelocity.x, tempVelocity.z));
		//if (speed > 0.0f)
		//{
		//	float drop = speed * groundFriction * dt;
		//	float newSpeed = speed - drop;
		//	if (newSpeed < 0.f)
		//		newSpeed = 0.f;

		//	if (speed > 0.f)
		//	{
		//		newSpeed /= speed;
		//		tempVelocity.x *= newSpeed;
		//		tempVelocity.z *= newSpeed;
		//	}
		//}
		// 

		// --- Apply Friction ---//
		float speed = glm::length(glm::vec2(tempVelocity.x, tempVelocity.z));
		bool hasInput = glm::length2(wishDir) > 0.0001f;

		if (speed > 0.0f)
		{
			float frictionScale = hasInput ? 0.3f : 1.0f;
			float drop = speed * groundFriction * frictionScale * dt;
			float newSpeed = glm::max(speed - drop, 0.f);

			if (speed > 0.f)
			{
				float ratio = newSpeed / speed;
				tempVelocity.x *= ratio;
				tempVelocity.z *= ratio;
			}
		}


		// --- Accelerate ---
		if (glm::length2(wishDir) > 0.0f)
		{
			float wishSpeed = maxGroundSpeed;

			float currentSpeed = glm::dot(
				glm::vec3(tempVelocity.x, 0.f, tempVelocity.z),
				wishDir
			);

			float addSpeed = wishSpeed - currentSpeed;

			if (addSpeed > 0.f)
			{
				float accelSpeed = groundAcceleration * wishSpeed * dt;
				if (accelSpeed > addSpeed)
					accelSpeed = addSpeed;

				tempVelocity += accelSpeed * wishDir;
			}
		}
	}

	// ==============================
	// AIR MOVEMENT
	// ==============================
	else
	{
		if (glm::length2(wishDir) > 0.0f)
		{
			float wishSpeed = maxAirSpeed;

			float currentSpeed = glm::dot(
				glm::vec3(tempVelocity.x, 0.f, tempVelocity.z),
				wishDir
			);

			float addSpeed = wishSpeed - currentSpeed;

			if (addSpeed > 0.f)
			{
				float accelSpeed = airAcceleration * wishSpeed * dt;
				if (accelSpeed > addSpeed)
					accelSpeed = addSpeed;

				tempVelocity += accelSpeed * wishDir;
			}
		}
	}

	// ==============================
	// HORIZONTAL SPEED CLAMP
	// ==============================
	glm::vec3 horizontal(tempVelocity.x, 0.f, tempVelocity.z);
	float horizontalSpeed = glm::length(horizontal);

	float maxSpeed = grounded ? maxGroundSpeed : maxAirSpeed;

	if (horizontalSpeed > maxSpeed)
	{
		horizontal = glm::normalize(horizontal) * maxSpeed;
		tempVelocity.x = horizontal.x;
		tempVelocity.z = horizontal.z;
	}

	// ==============================
	// JUMP
	// ==============================
	if (Input->IsKeyTriggered(keys::SPACE) && grounded)
	{
		tempVelocity.y = jumpForce;
	}

	// Apply final velocity directly
	glm::vec3 currentVel = playerRigidbody->velocity;
	glm::vec3 delta = tempVelocity - currentVel;

	physicsPtr->AddForce(
		playerRigidbody->actor,
		delta,
		ForceMode::VelocityChange
	);
}


inline void PlayerManagerScript::PlayerCameraControls() {
	auto* playerTransform = ecsPtr->GetComponent<ecs::TransformComponent>(entity);
	auto* playerCameraTransform = ecsPtr->GetComponent<ecs::TransformComponent>(playerCameraObjectID);
	auto* playerGunCameraTransform = ecsPtr->GetComponent<ecs::TransformComponent>(playerGunCameraObjectID);
	auto* playerGunModelPointTransform = ecsPtr->GetComponent<TransformComponent>(playerGunModelPointObjectID);

	if (!playerTransform || !playerCameraTransform || !playerGunCameraTransform || !playerGunModelPointTransform) {
		return;
	}

	float mouseRotationX = Input->GetAxisRaw("Mouse Y") * playerCameraSpeedX;
	float mouseRotationY = Input->GetAxisRaw("Mouse X") * playerCameraSpeedY;
	playerRotationX += mouseRotationX;
	playerRotationY += mouseRotationY;
	playerRotationX = glm::clamp(playerRotationX, -90.f, 90.f);

	//playerCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);
	//playerGunCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);


	// CAMERA TILT
	float horizontalInput = Input->GetHorizontal();

	float smoothSpeed = (std::abs(horizontalInput) > 0.05f) ? 10.f : 14.f;
	cameraTiltSmoothedInput = glm::mix(cameraTiltSmoothedInput, horizontalInput, glm::clamp(smoothSpeed * ecsPtr->m_GetDeltaTime(), 0.f, 1.f));

	float targetTiltZ = -horizontalInput * cameraTiltMaxAngle;

	float tiltBlend = (std::abs(horizontalInput) > 0.05f) ? cameraTiltSpeed : cameraTiltReturnSpeed;

	cameraCurrTiltZ = glm::mix(cameraCurrTiltZ, targetTiltZ, glm::clamp(tiltBlend * ecsPtr->m_GetDeltaTime(), 0.f, 1.f));
	// CAMERA SHAKE
	if (isCameraShaking) {
		cameraShakeElapsed += ecsPtr->m_GetDeltaTime();

		if (cameraShakeElapsed >= cameraShakeDelay) {
			float t = (cameraShakeElapsed - cameraShakeDelay) / cameraShakeDuration;

			if (t >= 1.f) {
				playerCameraTransform->LocalTransformation.position = cameraShakeOriginalPos;
				isCameraShaking = false;
				cameraShakeElapsed = 0.f;
				cameraShakeOffset = glm::vec3(0.f);
			}
			else {
				float curveValue = 1.f - t;
				float currentIntensity = cameraShakeIntensity * curveValue;

				auto randF = [](float range) -> float {
					return ((float)rand() / (float)RAND_MAX) * 2.f * range - range;
					};

				glm::vec3 randomOffset = glm::vec3(
					randF(currentIntensity),
					randF(currentIntensity),
					0.f
				);

				float yaw = glm::radians(playerRotationY);
				float pitch = glm::radians(playerRotationX);

				glm::mat4 rotMat = glm::rotate(glm::mat4(1.f), yaw, glm::vec3(0, 1, 0))
					* glm::rotate(glm::mat4(1.f), pitch, glm::vec3(1, 0, 0));

				cameraShakeOffset = glm::vec3(rotMat * glm::vec4(randomOffset, 0.f));
			}
		}
	}


	playerCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, cameraCurrTiltZ);
	playerGunCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, cameraCurrTiltZ);






	// PLAYER CROUCHING
	if (playerIsCrouching) {
		ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y = glm::mix(ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y, playerCrouchCameraPosY, playerCrouchTransitionSpeed * ecsPtr->m_GetDeltaTime());
	}
	else {
		ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y = glm::mix(ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y, originalPlayerCrouchCameraPosY, playerCrouchTransitionSpeed * ecsPtr->m_GetDeltaTime());
	}

	//if (cameraShakeTimer > 0.f) {
	//	playerCameraTransform->LocalTransformation.position += cameraShakeOffset;
	//}

	if (isCameraShaking && cameraShakeElapsed >= cameraShakeDelay) {
		playerCameraTransform->LocalTransformation.position = cameraShakeOriginalPos + cameraShakeOffset;
	}


	// PLAYER SPRINTING
	auto* cameraComp = ecsPtr->GetComponent<CameraComponent>(playerCameraObjectID);
	if (!cameraComp) {
		return;
	}

	if (playerIsSprinting) {
		cameraComp->fov = glm::mix(cameraComp->fov, playerSprintFOV, playerSprintFOVSpeed * ecsPtr->m_GetDeltaTime());
	}
	else {
		cameraComp->fov = glm::mix(cameraComp->fov, playerNormalFOV, playerSprintFOVSpeed * ecsPtr->m_GetDeltaTime());
	}

	// PLAYER WEAPON SWAYING
	auto* playerArmModelTransform = ecsPtr->GetComponent<TransformComponent>(playerArmModelObjectID);
	if (!playerArmModelTransform) {
		return;
	}

	//playerGunModelPointTransform->LocalTransformation.position = playerGunCameraTransform->LocalTransformation.position;

	gunRotationX = glm::mix(gunRotationX, -playerRotationX, playerGunModelSwaySpeed * ecsPtr->m_GetDeltaTime());
	gunRotationY = glm::mix(gunRotationY, -playerRotationY + 90.f, playerGunModelSwaySpeed * ecsPtr->m_GetDeltaTime());

	playerArmModelTransform->LocalTransformation.rotation = glm::vec3(gunRotationX, gunRotationY, 0.f);

	// PLAYER VIEW BOBBING
	glm::vec3 bobPosition = playerGunModelPointTransform->LocalTransformation.position;
	float playerGunModelBobbingSpeed = playerIsSprinting ? playerGunModelSprintBobbingSpeed : playerGunModelWalkBobbingSpeed;

	if (isMoving) {
		bobbingTimer += ecsPtr->m_GetDeltaTime() * playerGunModelBobbingSpeed;

		float offsetY = std::sin(bobbingTimer) * playerGunModelBobbingIntensity;
		bobPosition += glm::vec3(0.f, offsetY, 0.f);
	}
	else {
		bobbingTimer = 0.f;
	}

	playerArmModelTransform->LocalTransformation.position = glm::mix(playerArmModelTransform->LocalTransformation.position, bobPosition, ecsPtr->m_GetDeltaTime() * playerGunModelBobbingSpeed);
}

inline void PlayerManagerScript::PlayerCombatControls() {
	auto* cameraTransform = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID);

	if (!cameraTransform) {
		return;
	}

	// MANUAL TRANSFORM CHANGE FOR PROJECTILE POINT FOR NOW
	auto* projectilePointTransform = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID);
	if (!projectilePointTransform) {
		return;
	}

	bool playerIsAttacking = false;

	projectilePointTransform->LocalTransformation.position = cameraTransform->LocalTransformation.position + GetPlayerCameraFrontDirection() * 1.5f;

	//Animation Handling
	if (animComp)
	{
		// COMMENTED OUT FOR ANIM
		if (animComp->m_currentStateID)
		{
			R_Animation* currAnim = resource->GetResource<R_Animation>(playerController->RetrieveStateByID(animComp->m_currentStateID)->animationGUID).get();
			if (animComp->m_CurrentTime >= currAnim->GetDuration())
			{
				playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("animationFinished", animComp, playerController);
			}
		}
		
	}

	// SHOOT
	//if (Input->IsKeyTriggered(keys::LMB)) {
	//	playerIsAttacking = true;

	//	if (isReloading) return;

	//	// Cooldown check
	//	float& cd = GetCurrShootCooldownForCurrentWeapon();
	//	if (cd > 0.0f) return;

	//	// Ammo check
	//	int& currBullets = GetCurrBulletsForCurrentWeapon();
	//	if (currBullets <= 0) {
	//		if (autoReload) StartReload();
	//		return;
	//	}

	//	currBullets -= 1;
	//	cd = GetShootCooldownForCurrentWeapon();

	//	// COMMENTED OUT FOR ANIM
	//	
	//	if (animComp)
	//	{
	//		if (animComp->m_currentStateID)
	//		{
	//			//static_cast<AnimState*>(anim->m_currentState)->SetTrigger("hasShot");
	//			playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("hasShot", animComp, playerController);

	//		}
	//	}
		
		// ADD ATTACK ANIMATION HERE USING PLAYERISATTACKING BOOLEAN

		//if (playerPowerupHeld == Powerup::NONE) {
		//	std::shared_ptr<R_Scene> bullet = resource->GetResource<R_Scene>(bulletPrefab);

		//	if (bullet) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID bulletID = DuplicatePrefabIntoScene<R_Scene>(currentScene, bulletPrefab);

		//		if (auto* bulletTransform = ecsPtr->GetComponent<TransformComponent>(bulletID)) {
		//			bulletTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//		}

		//		if (auto* bulletScript = ecsPtr->GetComponent<BulletLogic>(bulletID)) {
		//			bulletScript->direction = GetPlayerCameraFrontDirection();
		//		}

		//		// GUN SFX
		//		if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
		//			std::vector<ecs::AudioFile*> playerHurtSfxPool;

		//			for (auto& af : ac->audioFiles) {
		//				if (af.isSFX) {
		//					playerHurtSfxPool.push_back(&af);
		//				}
		//			}

		//			if (!playerHurtSfxPool.empty()) {
		//				int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
		//				//std::cout << "[BulletLogic] Random SFX index chosen = " << idx << std::endl;

		//				playerHurtSfxPool[idx]->requestPlay = true;
		//			}
		//		}
		//	}
		//}
		//else if (playerPowerupHeld == Powerup::FIRE) {
		//	std::shared_ptr<R_Scene> fireball = resource->GetResource<R_Scene>(firePrefab);

		//	if (fireball) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID fireballID = DuplicatePrefabIntoScene<R_Scene>(currentScene, firePrefab);

		//		if (auto* fireballTransform = ecsPtr->GetComponent<TransformComponent>(fireballID)) {
		//			fireballTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//		}

		//		if (auto* fireballScript = ecsPtr->GetComponent<FirePowerupManagerScript>(fireballID)) {
		//			fireballScript->direction = GetPlayerCameraFrontDirection();
		//		}

		//		playerPowerupHeld = Powerup::NONE;
		//	}
		//}
		//else if (playerPowerupHeld == Powerup::ACID) {
		//	std::shared_ptr<R_Scene> acidBlast = resource->GetResource<R_Scene>(acidPrefab);

		//	if (acidBlast) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID acidBlastID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidPrefab);

		//		if (auto* acidBlastTransform = ecsPtr->GetComponent<TransformComponent>(acidBlastID)) {
		//			acidBlastTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//		}

		//		if (auto* acidBlastScript = ecsPtr->GetComponent<AcidPowerupManagerScript>(acidBlastID)) {
		//			acidBlastScript->direction = GetPlayerCameraFrontDirection();
		//		}

		//		playerPowerupHeld = Powerup::NONE;
		//	}
		//}
		//else if (playerPowerupHeld == Powerup::LIGHTNING) {
		//	///Lightning missing range variable
		//	/*std::shared_ptr<R_Scene> lightningStrike = resource->GetResource<R_Scene>(lightningPrefab);

		//	if (lightningStrike) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID lightningStrikeID = DuplicatePrefabIntoScene<R_Scene>(currentScene, lightningPrefab);

		//		if (auto* lightningStrikeTransform = ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)) {
		//			RaycastHit hit;
		//			hit.entityID = 9999999;
		//			glm::vec3 dir = GetPlayerCameraFrontDirection();

		//			float range = ecsPtr->GetComponent<LightningPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)->m_childID[0])->range;
		//			physicsPtr->Raycast(cameraTransform->WorldTransformation.position, dir, range, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);

		//			if (hit.entityID != 9999999) {
		//				lightningStrikeTransform->LocalTransformation.position = hit.point;

		//				playerPowerupHeld = Powerup::NONE;
		//			}

		//		}
		//	}*/
		//}
		//else if (playerPowerupHeld == Powerup::FIREACID) {
		//	std::shared_ptr<R_Scene> flamethrower = resource->GetResource<R_Scene>(fireAcidPrefab);

		//	if (flamethrower) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID flamethrowerID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireAcidPrefab);

		//		if (auto* flamethrowerTransform = ecsPtr->GetComponent<TransformComponent>(flamethrowerID)) {
		//			//flamethrowerTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//			ecsPtr->SetParent(playerProjectilePointObjectID, flamethrowerID);
		//			flamethrowerTransform->LocalTransformation.position = glm::vec3(0.f, 0.f, 0.f);

		//			// TODO: SOMEHOW MAKE THE FLAMETHROWER ABIDE TO THE PLAYER'S ROTATION
		//			//flamethrowerTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);
		//		}
		//	}

		//	playerPowerupHeld = Powerup::NONE;
		//}
		//else if (playerPowerupHeld == Powerup::FIRELIGHTNING) {
		//	std::shared_ptr<R_Scene> groundSpikes = resource->GetResource<R_Scene>(fireLightningPrefab);

		//	if (groundSpikes) {
		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID groundSpikesID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireLightningPrefab);

		//		if (auto* groundSpikesTransform = ecsPtr->GetComponent<TransformComponent>(groundSpikesID)) {
		//			groundSpikesTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(entity)->WorldTransformation.position;
		//		}

		//		playerPowerupHeld = Powerup::NONE;
		//	}
		//}
		//else if (playerPowerupHeld == Powerup::ACIDLIGHTNING) {
		//	std::shared_ptr<R_Scene> starfall = resource->GetResource<R_Scene>(acidLightningPrefab);

		//	if (starfall) {
		//		// Update later
		//		//std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		//ecs::EntityID starfallID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidLightningPrefab);

		//		//if (auto* starfallTransform = ecsPtr->GetComponent<TransformComponent>(starfallID)) {
		//		//	starfallTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//		//}

		//		//glm::vec3 dir = GetPlayerCameraFrontDirection();
		//		//for (int i = 0; i < ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID.size(); ++i) {
		//		//	// TODO: RANDOMIZE THE DIRECTION OF THE STARFALL PROJECTILE HERE OR SOMETHING
		//		//	ecsPtr->GetComponent<LightningAcidPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID[i])->direction = dir;
		//		//}

		//		//playerPowerupHeld = Powerup::NONE;

		//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
		//		ecs::EntityID fireballID = DuplicatePrefabIntoScene<R_Scene>(currentScene, firePrefab);

		//		if (auto* fireballTransform = ecsPtr->GetComponent<TransformComponent>(fireballID)) {
		//			fireballTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
		//		}

		//		if (auto* fireballScript = ecsPtr->GetComponent<FirePowerupManagerScript>(fireballID)) {
		//			fireballScript->direction = GetPlayerCameraFrontDirection();
		//		}

		//		playerPowerupHeld = Powerup::NONE;
		//	}
		//}
	//}

	// INTERACT
	//if (Input->IsKeyTriggered(keys::RMB)) {

	//	bool hasAbsorbed = false;


	//	RaycastHit hit;
	//	hit.entityID = 9999999;
	//	physicsPtr->Raycast(cameraTransform->WorldTransformation.position, GetPlayerCameraFrontDirection(), interactPowerupRange, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);

	//	if (hit.entityID != 9999999 && ecsPtr->GetComponent<NameComponent>(hit.entityID)->entityTag == "Powerup") {
	//		if (auto* powerupComp = ecsPtr->GetComponent<PowerupManagerScript>(hit.entityID)) {
	//			hasAbsorbed = true;
	//			if (playerPowerupHeld == Powerup::NONE) {
	//				if (powerupComp->powerupType == "FIRE") {
	//					playerPowerupHeld = Powerup::FIRE;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "ACID") {
	//					playerPowerupHeld = Powerup::ACID;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "LIGHTNING") {
	//					playerPowerupHeld = Powerup::LIGHTNING;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//			}
	//			else if (playerPowerupHeld == Powerup::FIRE) {
	//				if (powerupComp->powerupType == "FIRE") {
	//					playerPowerupHeld = Powerup::FIRE;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "ACID") {
	//					///playerPowerupHeld = Powerup::FIREACID;
	//					playerPowerupHeld = Powerup::FIRE;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "LIGHTNING") {
	//					///playerPowerupHeld = Powerup::FIRELIGHTNING;
	//					playerPowerupHeld = Powerup::FIRE;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//			}
	//			else if (playerPowerupHeld == Powerup::ACID) {
	//				if (powerupComp->powerupType == "FIRE") {
	//					///playerPowerupHeld = Powerup::FIREACID;
	//					playerPowerupHeld = Powerup::ACID;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "ACID") {
	//					playerPowerupHeld = Powerup::ACID;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "LIGHTNING") {
	//					///playerPowerupHeld = Powerup::ACIDLIGHTNING;
	//					playerPowerupHeld = Powerup::ACID;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//			}
	//			else if (playerPowerupHeld == Powerup::LIGHTNING) {
	//				if (powerupComp->powerupType == "FIRE") {
	//					///playerPowerupHeld = Powerup::FIRELIGHTNING;
	//					playerPowerupHeld = Powerup::LIGHTNING;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "ACID") {
	//					///playerPowerupHeld = Powerup::ACIDLIGHTNING;
	//					playerPowerupHeld = Powerup::LIGHTNING;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//				else if (powerupComp->powerupType == "LIGHTNING") {
	//					playerPowerupHeld = Powerup::LIGHTNING;

	//					// ADD PARTICLE EFFECT HERE
	//				}
	//			}

	//			// ADD SFX OF POWERUP PICKUP HERE
	//		}
	//	}

	//	// COMMENTED OUT FOR ANIM
	//	
	//	if (animComp && hasAbsorbed)
	//	{
	//		if (animComp->m_currentStateID)
	//		{
	//			playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("hasAbsorbed", animComp, playerController);
	//			hasAbsorbed = false;
	//		}
	//	}
	//	
	//}

	// MANA
	if (playerPowerupHeld != Powerup::NONE){
		currMana -= ecsPtr->m_GetDeltaTime();

		if (currMana <= 0.0f){
			currMana = 0.0f;
			playerPowerupHeld = Powerup::NONE;
		}
	}

	// INTERACT
	if (Input->IsKeyTriggered(keys::E)) {

		if (currMana > 0.0f || playerPowerupHeld != Powerup::NONE)
			return;

		bool hasAbsorbed = false;


		RaycastHit hit;
		hit.entityID = 9999999;
		physicsPtr->Raycast(cameraTransform->WorldTransformation.position, GetPlayerCameraFrontDirection(), interactPowerupRange, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);

		if (hit.entityID != 9999999 && ecsPtr->GetComponent<NameComponent>(hit.entityID)->entityTag == "Powerup") {
			if (auto* powerupComp = ecsPtr->GetComponent<PowerupManagerScript>(hit.entityID)) {
				hasAbsorbed = true;

				if (powerupComp->powerupType == "FIRE") {
					playerPowerupHeld = Powerup::FIRE;
				}
				else if (powerupComp->powerupType == "ACID") {
					playerPowerupHeld = Powerup::ACID;
				}
				else if (powerupComp->powerupType == "LIGHTNING") {
					playerPowerupHeld = Powerup::LIGHTNING;
				}
				
				currMana = maxMana;
			/*	currInteractCooldown = interactCooldown;
				std::cout << "Powerup picked up. Cooldown STARTO!!!!::: "
					<< currInteractCooldown << "s\n";*/

				// ADD SFX

				if (animComp && hasAbsorbed)
				{
					if (animComp->m_currentStateID)
					{
						playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("hasAbsorbed", animComp, playerController);
						hasAbsorbed = false;
					}
				}
			}
		}
	}

	// SHOOT
	// ADD RELOAD HERE
	if (Input->IsKeyTriggered(keys::LMB)) {

		// Don't shoot while reloading
		if (isReloading) return;

		// Cooldown check
		float& cd = GetCurrShootCooldownForCurrentWeapon();
		if (cd > 0.0f) return;

		// Ammo check
		int& currBullets = GetCurrBulletsForCurrentWeapon();
		if (currBullets <= 0) {
			if (autoReload) StartReload();
			return;
		}

		// Consume ammo + apply cooldown
		currBullets -= 1;
		cd = GetShootCooldownForCurrentWeapon();

		if (animComp)
		{
			if (animComp->m_currentStateID)
			{
				//static_cast<AnimState*>(anim->m_currentState)->SetTrigger("hasShot");
				playerController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("hasShot", animComp, playerController);

			}
		}

		if (playerPowerupHeld == Powerup::NONE) {
			std::shared_ptr<R_Scene> bullet = resource->GetResource<R_Scene>(bulletPrefab);

			if (bullet) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID bulletID = DuplicatePrefabIntoScene<R_Scene>(currentScene, bulletPrefab);

				if (auto* bulletTransform = ecsPtr->GetComponent<TransformComponent>(bulletID)) {
					bulletTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				}

				if (auto* bulletScript = ecsPtr->GetComponent<BulletLogic>(bulletID)) {
					bulletScript->direction = GetPlayerCameraFrontDirection();
				}

				// GUN SFX
				//if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
				//	std::vector<ecs::AudioFile*> playerHurtSfxPool;

				//	for (auto& af : ac->audioFiles) {
				//		if (af.isSFX) {
				//			playerHurtSfxPool.push_back(&af);
				//		}
				//	}

				//	if (!playerHurtSfxPool.empty()) {
				//		int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
				//		//std::cout << "[BulletLogic] Random SFX index chosen = " << idx << std::endl;

				//		playerHurtSfxPool[idx]->requestPlay = true;
				//	}
				//}
				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

					for (auto& af : ac->audioFiles) {
						if (af.audioGUID == gunSfxGUID_1 && af.isSFX) {
							af.requestPlay = true;
							break;
						}
					}
				}



			}
		}
		else if (playerPowerupHeld == Powerup::FIRE) {

			if (fireCurrMeleeCooldown > 0.0f)
				return;

			std::shared_ptr<R_Scene> fireLMB = resource->GetResource<R_Scene>(fireLMBPrefab);

			fireCurrMeleeCooldown = fireMeleeCooldown;

			if (fireLMB) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID fireLMBID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireLMBPrefab);

				//if (auto* fireLMBTransform = ecsPtr->GetComponent<TransformComponent>(fireLMBID)) {
				//	fireLMBTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				//}

				////if (auto* fireLMBScript = ecsPtr->GetComponent<FireLMB>(fireLMBID)) {
				////	//fireLMBScript->direction = glm::vec3(0.f);
				////}

				//if (auto* fireLMBScript = ecsPtr->GetComponent<FireLMB>(fireLMBID)) {
				//	glm::vec3 dir = GetPlayerCameraFrontDirection();
				//	dir.y = 0.f;
				//	if (glm::length(dir) > 0.0001f)
				//		dir = glm::normalize(dir);

				//	fireLMBScript->direction = dir;
				//}

				//ecs::EntityID fireLMBID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireLMBPrefab);

				auto* spawnTf = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID);
				auto* fireTf = ecsPtr->GetComponent<TransformComponent>(fireLMBID);
				if (!spawnTf || !fireTf) return;

				glm::vec3 dir = GetPlayerCameraFrontDirection();
				dir.y = 0.f;
				if (glm::length(dir) > 0.0001f) dir = glm::normalize(dir);

				fireTf->LocalTransformation.position = spawnTf->WorldTransformation.position;
				float yaw = glm::degrees(std::atan2(dir.x, dir.z));
				fireTf->LocalTransformation.rotation = glm::vec3(0.f, yaw, 0.f);

				if (auto* fireLMBScript = ecsPtr->GetComponent<FireLMB>(fireLMBID)) {
					fireLMBScript->direction = dir; 
				}

				if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

					for (auto& af : ac->audioFiles) {
						if (af.audioGUID == fireSlashSfxGUID && af.isSFX) {
							af.requestPlay = true;
							break;
						}
					}
				}

			}


			// ADD SFX
		}
		else if (playerPowerupHeld == Powerup::ACID) {
			std::shared_ptr<R_Scene> acidLMB = resource->GetResource<R_Scene>(acidLMBPrefab);

			if (acidLMB) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID acidLMBID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidLMBPrefab);

				auto* spawnTf = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID);
				auto* acidLMBTf = ecsPtr->GetComponent<TransformComponent>(acidLMBID);
				if (!spawnTf || !acidLMBTf) return;

				acidLMBTf->LocalTransformation.position = spawnTf->WorldTransformation.position;

				if (auto* acidLMBScript = ecsPtr->GetComponent<AcidLMB>(acidLMBID)) {
					glm::vec3 launchVel = GetPlayerCameraFrontDirection() * acidLMBScript->launchSpeed;
					launchVel.y += acidLMBScript->arcUpwardKick;
					acidLMBScript->velocity = launchVel;
				}
			}
			// ADD SFX
}

		else if (playerPowerupHeld == Powerup::LIGHTNING) {
			std::shared_ptr<R_Scene> lightningLMB = resource->GetResource<R_Scene>(lightningLMBPrefab);

			if (lightningLMB) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID lightningLMBID = DuplicatePrefabIntoScene<R_Scene>(currentScene, lightningLMBPrefab);

				if (auto* lightningLMBTransform = ecsPtr->GetComponent<TransformComponent>(lightningLMBID)) {
					lightningLMBTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				}

				if (auto* lightningLMBScript = ecsPtr->GetComponent<LightningLMB>(lightningLMBID)) {
					lightningLMBScript->direction = GetPlayerCameraFrontDirection();
				}
			}

			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

				for (auto& af : ac->audioFiles) {
					if (af.audioGUID == lightningGunSfxGUID && af.isSFX) {
						af.requestPlay = true;
						break;
					}
				}
			}

			ecsPtr->SetTimeScale(0.5f);


		}
	}

	// ABILITY
	if (Input->IsKeyTriggered(keys::RMB)) {
		if (playerPowerupHeld == Powerup::FIRE ) {
			std::shared_ptr<R_Scene> fireball = resource->GetResource<R_Scene>(firePrefab);

			if (fireball) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID fireballID = DuplicatePrefabIntoScene<R_Scene>(currentScene, firePrefab);

				if (auto* fireballTransform = ecsPtr->GetComponent<TransformComponent>(fireballID)) {
					fireballTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				}

				if (auto* fireballScript = ecsPtr->GetComponent<FirePowerupManagerScript>(fireballID)) {
					fireballScript->direction = GetPlayerCameraFrontDirection();
				}

				currMana -= fireAbilityCost;
			}

			// ADD SFX
		}

		//Acid Blast
		else if (playerPowerupHeld == Powerup::ACID) {

			if (currMana < acidAbilityCost) return;

			std::shared_ptr<R_Scene> airBlast = resource->GetResource<R_Scene>(airBlastPrefab);

			if (airBlast) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID airBlastID = DuplicatePrefabIntoScene<R_Scene>(currentScene, airBlastPrefab);

				if (auto* airBlastTransform = ecsPtr->GetComponent<TransformComponent>(airBlastID)) {
					airBlastTransform->LocalTransformation.position =
						ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				}

				currMana -= acidAbilityCost;

				std::cout << "[AirBlast] Spawned | Mana left: " << currMana << "\n";
			}

			// ADD SFX
		}
		else if (playerPowerupHeld == Powerup::LIGHTNING ) {
			std::shared_ptr<R_Scene> railgun = resource->GetResource<R_Scene>(lightningPrefab);

			if (railgun) {
				std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
				ecs::EntityID railgunID = DuplicatePrefabIntoScene<R_Scene>(currentScene, lightningPrefab);

				if (auto* railgunTransform = ecsPtr->GetComponent<TransformComponent>(railgunID)) {
					railgunTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
				}

				//auto test = ecsPtr->GetChild(railgunID);
				//ecs::EntityID first;
				//if (test.has_value() && !test->empty())
				//{
				//	first = test.value().front();
				//}

				//if (auto* railgunScript = ecsPtr->GetComponent<LightningPowerupManagerScript>(first)) {
				//	railgunScript->direction = GetPlayerCameraFrontDirection();
				//	
				//}

				if (auto* railgunScript = ecsPtr->GetComponent<LightningPowerupManagerScript>(railgunID)) {
					railgunScript->direction = GetPlayerCameraFrontDirection();
				}

				currMana -= lightningAbilityCost;
			}

			// ADD SFX
		}
	}

	// MOVEMENT
	if (Input->IsKeyTriggered(keys::F)) {
		auto* playerRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity);
		if (!playerRigidbody) return;

		if (playerPowerupHeld == Powerup::FIRE && fireCurrMovementCooldown <= 0.f) {
		
			std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			ecs::EntityID fireDashID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireDashPrefab);
			ecs::EntityID parentID = entity;
			ecsPtr->SetParent(parentID, fireDashID, false);
			if (auto* vfxTf = ecsPtr->GetComponent<TransformComponent>(fireDashID))
			{
				vfxTf->LocalTransformation.position = glm::vec3(1.f, 1.f, 0.f);  // offset
				vfxTf->LocalTransformation.rotation = glm::vec3(0.f, 0.f, 0.f);
			}

			//fireDashVfxTimer = fireDashVfxDuration;
			//ecsPtr->SetActive(fireDashID, true);

			physicsPtr->AddForce(playerRigidbody->actor, GetPlayerFrontDirection() * 50.f, ForceMode::Impulse);

			isDashing = true;
			currentDashTimer = dashDuration;

			currMana -= fireMovementCost;
			fireCurrMovementCooldown = fireMovementCooldown;

			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

				for (auto& af : ac->audioFiles) {
					if (af.audioGUID == fireDashSfxGUID && af.isSFX) {
						af.requestPlay = true;
						break;
					}
				}
			}
			// ADD SFX
		}
		
		//Acid SHield
		else if (playerPowerupHeld == Powerup::ACID && acidCurrShieldCooldown <= 0.f) {
			if (currMana < acidShieldCost) return;

			std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			ecs::EntityID acidShieldID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidShieldPrefab);

			ecsPtr->SetParent(entity, acidShieldID, false);

			if (auto* shieldTf = ecsPtr->GetComponent<TransformComponent>(acidShieldID)) {
				shieldTf->LocalTransformation.position = glm::vec3(0.f, 0.f, 0.f);
			}

			currMana -= acidShieldCost;
			acidCurrShieldCooldown = acidShieldCooldown;

			//physicsPtr->AddForce(playerRigidbody->actor, GetPlayerFrontDirection() * 25.f, ForceMode::Impulse);

			//currMana -= acidMovementCost;
			//acidCurrMovementCooldown = acidMovementCooldown;

			// ADD SFX
		}

		//Time slow
		else if (playerPowerupHeld == Powerup::LIGHTNING) {
			//Add VFX here?

			if (lightningCurrTimeslowCooldown > 0.f) return;
			if (currMana < lightningTimeslowCost)    return;
			if (isTimeslowActive)                    return;

			ecsPtr->SetTimeScale(0.5f);

			isTimeslowActive = true;

			lightningCurrTimeslowTimer = lightningTimeslowDuration;
			lightningCurrTimeslowCooldown = lightningTimeslowCooldown;

			currMana -= lightningTimeslowCost;

			// ADD SFX HERE 
			if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

				for (auto& af : ac->audioFiles) {
					if (af.audioGUID == lightningDashSfxGUID && af.isSFX) {
						af.requestPlay = true;
						break;
					}
				}
			}
		}

	}
}


inline bool PlayerManagerScript::GroundCheck() {
	if (auto* groundCheck = ecsPtr->GetComponent<GroundCheckScript>(playerGroundCheckObjectID)) {
		return groundCheck->groundCheck;
	}

	return false;
}

inline void PlayerManagerScript::UpdateHealthUI() {
	if (auto* text = ecsPtr->GetComponent<TextComponent>(healthUIObjectID)) {
		text->text = "HP: " + std::to_string(currPlayerHitPoints) + "/" + std::to_string(maxPlayerHitPoints);
	}
}

// HELPER FUNCTIONS
inline glm::vec3 PlayerManagerScript::GetPlayerCameraFrontDirection() {
	float yaw = glm::radians(playerRotationY);
	float pitch = glm::radians(playerRotationX);

	glm::vec3 dir;
	dir.x = std::cos(pitch) * std::cos(yaw);
	dir.y = std::sin(pitch);
	dir.z = std::cos(pitch) * std::sin(yaw);

	dir = glm::normalize(dir);

	return dir;
}

inline glm::vec3 PlayerManagerScript::GetPlayerCameraRightDirection() {
	float yaw = glm::radians(playerRotationY);

	glm::vec3 dir;
	dir.x = std::sin(yaw);
	dir.y = 0.f;
	dir.z = -std::sin(yaw);

	dir = glm::normalize(dir);

	return dir;
}

inline glm::vec3 PlayerManagerScript::GetPlayerCameraUpDirection() {
	float yaw = glm::radians(playerRotationY);
	float pitch = glm::radians(playerRotationX);

	glm::vec3 dir;
	dir.x = -std::cos(pitch) * std::cos(yaw);
	dir.y = std::cos(pitch);
	dir.z = -std::sin(pitch) * std::sin(yaw);

	dir = glm::normalize(dir);

	return dir;
}

inline glm::vec3 PlayerManagerScript::GetPlayerFrontDirection() {
	float cameraRotationY = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.rotation.y;
	glm::vec3 dir(std::sin(glm::radians(-cameraRotationY + 90.f)), 0.f, std::cos(glm::radians(-cameraRotationY + 90.f)));

	return dir;
}

inline glm::vec3 PlayerManagerScript::GetPlayerRightDirection() {
	float cameraRotationY = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.rotation.y;
	glm::vec3 dir(std::sin(glm::radians(-cameraRotationY)), 0.f, std::cos(glm::radians(-cameraRotationY)));

	return dir;
}

inline void PlayerManagerScript::CameraShake(float intensity, float duration) {
	// Save curr position
	auto* cam = ecsPtr->GetComponent<ecs::TransformComponent>(playerCameraObjectID);
	if (!cam) return;

	// If already shaking, restore first 
	if (isCameraShaking) {
		cam->LocalTransformation.position = cameraShakeOriginalPos;
	}

	cameraShakeOriginalPos = cam->LocalTransformation.position;
	cameraShakeIntensity = intensity;
	cameraShakeDuration = duration;
	cameraShakeDelay = 0.f;  
	cameraShakeElapsed = 0.f;
	cameraShakeOffset = glm::vec3(0.f);
	isCameraShaking = true;
}
