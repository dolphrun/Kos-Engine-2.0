#pragma once
#include "TemplateSC.h"
#include "PauseMenuScript.h"
#include "LoseScreenScript.h"
#include "WinScreenScript.h"

class PlayerManagerScript : public TemplateSC {
public:

	R_AnimController* playerController = nullptr;
	AnimatorComponent* anim = nullptr;
	AnimState currAnimationState;

	enum Powerup {
		NONE = 0,
		FIRE = 1,
		ACID = 2,
		LIGHTNING = 3,
		FIREACID = 4,
		FIRELIGHTNING = 5,
		ACIDLIGHTNING = 6
	};

	// PLAYER DETAILS
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
	float playerGunModelWalkBobbingSpeed = 2.5f;
	float playerGunModelSprintBobbingSpeed = 5.f;
	float playerGunModelBobbingIntensity = 0.015f;

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

	utility::GUID bulletPrefab;

	utility::GUID firePrefab;
	utility::GUID acidPrefab;
	utility::GUID lightningPrefab;

	utility::GUID fireAcidPrefab;
	utility::GUID fireLightningPrefab;
	utility::GUID acidLightningPrefab;

	// BACKEND PLAYER DETAILS
	float playerRotationX = 0.f, playerRotationY = 0.f;

	bool playerIsWalking = false;
	bool playerIsSprinting = false;
	bool playerIsCrouching = false;
	bool playerIsSliding = false;

	float originalPlayerCrouchCameraPosY;
	float playerCrouchCameraPosY;

	float gunRotationY = 90.f;
	float gunRotationX = 0.f;

	float bobbingTimer = 0.f;

	// SFX
	utility::GUID gunSfxGUID_1;
	utility::GUID gunSfxGUID_2;

	void Start() override {
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

		if (pauseMenuManagerObject != utility::GUID{}) {
			pauseMenuManagerID = ecsPtr->GetEntityIDFromGUID(pauseMenuManagerObject);
			std::cout << "PlayerManager connected to PauseMenuManager!\n";
		}
		healthUIObjectID = ecsPtr->GetEntityIDFromGUID(healthUIObject);
		loseScreenCanvasID = ecsPtr->GetEntityIDFromGUID(loseScreenCanvasObject);
		winScreenCanvasID = ecsPtr->GetEntityIDFromGUID(winScreenCanvasObject);

		std::vector<EntityID> armChild = ecsPtr->GetChild(playerArmModelObjectID).value();
		if (anim = ecsPtr->GetComponent<ecs::AnimatorComponent>(armChild[0]))
		{
			playerController = resource->GetResource<R_AnimController>(anim->controllerGUID).get();
			if (playerController)
			{
				/*currAnimationState = *playerController->m_EnterState;
				anim->m_currentState = &currAnimationState;
				static_cast<AnimState*>(anim->m_currentState)->SetTrigger("ForcedEntry");*/
			}
		}

	}

	void Update() override {

		if (Input->IsKeyReleased(keys::L)) {
			std::cout << "L RELEASED\n";
			Scenes->ReloadScene();

			if (auto* pauseManager = ecsPtr->GetComponent<PauseMenuScript>(pauseMenuManagerID)) {
				if (pauseManager->isPaused) {
					pauseManager->TogglePause();
				}
			}

		}

		if (Input->IsKeyTriggered(keys::ESC)) {
			if (auto* pauseManager = ecsPtr->GetComponent<PauseMenuScript>(pauseMenuManagerID)) {
				std::cout << "PAUSE PAUSE PAUSE\n";
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

		PlayerMovementControls();
		PlayerCameraControls();
		PlayerCombatControls();

		if (Input->IsKeyPressed(keys::SPACE)) {
			std::cout << "[DEBUG] Space pressed - Grounded: " << GroundCheck() << std::endl;
		}
		//if (Input->IsKeyTriggered(keys::UP)) {
		//	std::cout << "Player takes 1 damage for testing purposes.\n"; 
		//	TakeDamage(1);
		//}
	}

	void FixedUpdate() override {

	}

	void PlayerMovementControls() {
		auto* playerRigidbody = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity);
		if (!playerRigidbody) return;

		// Calculate current horizontal velocity
		glm::vec3 currentVelocity = playerRigidbody->velocity;
		glm::vec3 horizontalVelocity = glm::vec3(currentVelocity.x, 0.f, currentVelocity.z);
		float currentSpeed = glm::length(horizontalVelocity);

		// Determine target speed based on movement state
		float targetSpeed = 0.f;
		bool isMoving = (std::abs(Input->GetHorizontal()) > 0.1f || std::abs(Input->GetVertical()) > 0.1f);

		// Reset flags
		playerIsWalking = false;
		playerIsSprinting = false;

		// SPRINTING
		if (Input->IsKeyPressed(keys::LeftShift) && Input->GetVertical() > 0.f && GroundCheck() && !playerIsCrouching) {
			playerIsSprinting = true;
			targetSpeed = maxPlayerMovSpeed * playerSprintMultiplier; // 15 m/s
		}

		// CROUCHING (not sliding)
		else if (playerIsCrouching && !playerIsSliding) {
			targetSpeed = maxPlayerMovSpeed * playerCrouchMultiplier; // 5 m/s
		}

		// WALKING
		else if (isMoving) {
			playerIsWalking = true;
			targetSpeed = maxPlayerMovSpeed; // 10 m/s
		}

		// CROUCH/SLIDE HANDLING
		if (Input->IsKeyPressed(keys::LeftControl) && !playerIsSliding && GroundCheck()) {
			if (!playerIsCrouching) {
				playerIsCrouching = true;

				// Check if we should slide
				if (currentSpeed >= playerVelocityBeforeSlide) {
					playerIsSliding = true;

					// Apply slide impulse in current movement direction
					glm::vec3 slideDirection = glm::length(horizontalVelocity) > 0.1f ?
						glm::normalize(horizontalVelocity) :
						GetPlayerFrontDirection();

					glm::vec3 slideForce = slideDirection * playerSlideMultiplier;
					physicsPtr->AddForce(playerRigidbody->actor, slideForce, ForceMode::Impulse);
				}
			}
		}

		else if (Input->IsKeyReleased(keys::LeftControl)) {
			playerIsCrouching = false;
			playerIsSliding = false;
		}

		// MOVEMENT (don't apply if sliding - let physics handle it)
		if (!playerIsSliding && isMoving) {
			glm::vec3 inputDirection = GetPlayerFrontDirection() * Input->GetVertical() +
				GetPlayerRightDirection() * Input->GetHorizontal();

			if (glm::length2(inputDirection) > glm::epsilon<float>()) {
				inputDirection = glm::normalize(inputDirection);

				// Calculate desired velocity
				glm::vec3 desiredVelocity = inputDirection * targetSpeed;

				// Calculate the force needed (proportional to velocity difference)
				glm::vec3 velocityError = desiredVelocity - horizontalVelocity;

				// Apply acceleration force (this value controls responsiveness)
				float groundAcceleration = 25.f; // Units/s² - adjust for feel
				glm::vec3 moveForce = velocityError * groundAcceleration;

				physicsPtr->AddForce(playerRigidbody->actor, moveForce, ForceMode::Acceleration);
			}
		}

		// STOPPING - apply friction when no input and on ground
		else if (!playerIsSliding && GroundCheck() && !isMoving && currentSpeed > 0.1f) {
			float stopDeceleration = 20.f; // How fast to stop
			glm::vec3 stopForce = -glm::normalize(horizontalVelocity) * stopDeceleration;
			physicsPtr->AddForce(playerRigidbody->actor, stopForce, ForceMode::Acceleration);
		}

		// SLIDING FRICTION - slow down while sliding
		if (playerIsSliding) {
			float slideFriction = 10.f; // How fast slide decays
			if (currentSpeed > maxPlayerMovSpeed * playerCrouchMultiplier) {
				glm::vec3 frictionForce = -glm::normalize(horizontalVelocity) * slideFriction;
				physicsPtr->AddForce(playerRigidbody->actor, frictionForce, ForceMode::Acceleration);
			}
			else {
				// Stop sliding when we slow down enough
				playerIsSliding = false;
			}
		}

		// JUMPING
		if (Input->IsKeyTriggered(keys::SPACE) && GroundCheck()) {
			glm::vec3 jumpVelocity(0.f, currPlayerJumpForce, 0.f);
			physicsPtr->AddForce(playerRigidbody->actor, jumpVelocity, ForceMode::VelocityChange);
		}
	}

	void PlayerCameraControls() {
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

		playerCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);
		playerGunCameraTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);

		// PLAYER CROUCHING
		if (playerIsCrouching) {
			ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y = glm::mix(ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y, playerCrouchCameraPosY, playerCrouchTransitionSpeed * ecsPtr->m_GetDeltaTime());
		}
		else {
			ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y = glm::mix(ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.position.y, originalPlayerCrouchCameraPosY, playerCrouchTransitionSpeed * ecsPtr->m_GetDeltaTime());
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

		if (playerIsWalking || playerIsSprinting) {
			bobbingTimer += ecsPtr->m_GetDeltaTime() * playerGunModelBobbingSpeed;

			float offsetY = std::sin(bobbingTimer) * playerGunModelBobbingIntensity;
			bobPosition += glm::vec3(0.f, offsetY, 0.f);
		}
		else {
			bobbingTimer = 0.f;
		}

		playerArmModelTransform->LocalTransformation.position = glm::mix(playerArmModelTransform->LocalTransformation.position, bobPosition, ecsPtr->m_GetDeltaTime() * playerGunModelBobbingSpeed);
	}

	void PlayerCombatControls() {
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
		if (anim)
		{
			if (anim->m_currentStateID)
			{

				/*R_Animation* currAnim = resource->GetResource<R_Animation>(static_cast<AnimState*>(anim->m_currentState)->animationGUID).get();
				if (anim->m_CurrentTime >= currAnim->GetDuration())
				{
					static_cast<AnimState*>(anim->m_currentState)->SetTrigger("animationFinished");
					anim->m_CurrentTime = 0.f;
				}*/

			}
		}

		// SHOOT
		if (Input->IsKeyTriggered(keys::LMB)) {
			playerIsAttacking = true;

			if (anim)
			{
				if (anim->m_currentStateID)
				{
					//static_cast<AnimState*>(anim->m_currentState)->SetTrigger("hasShot");
				}
			}
			// ADD ATTACK ANIMATION HERE USING PLAYERISATTACKING BOOLEAN

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
					if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
						std::vector<ecs::AudioFile*> playerHurtSfxPool;

						for (auto& af : ac->audioFiles) {
							if (af.isSFX) {
								playerHurtSfxPool.push_back(&af);
							}
						}

						if (!playerHurtSfxPool.empty()) {
							int idx = rand() % static_cast<int>(playerHurtSfxPool.size());
							std::cout << "[BulletLogic] Random SFX index chosen = " << idx << std::endl;

							playerHurtSfxPool[idx]->requestPlay = true;
						}
					}
				}
			}
			else if (playerPowerupHeld == Powerup::FIRE) {
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

					playerPowerupHeld = Powerup::NONE;
				}
			}
			else if (playerPowerupHeld == Powerup::ACID) {
				std::shared_ptr<R_Scene> acidBlast = resource->GetResource<R_Scene>(acidPrefab);

				if (acidBlast) {
					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					ecs::EntityID acidBlastID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidPrefab);

					if (auto* acidBlastTransform = ecsPtr->GetComponent<TransformComponent>(acidBlastID)) {
						acidBlastTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
					}

					if (auto* acidBlastScript = ecsPtr->GetComponent<AcidPowerupManagerScript>(acidBlastID)) {
						acidBlastScript->direction = GetPlayerCameraFrontDirection();
					}

					playerPowerupHeld = Powerup::NONE;
				}
			}
			else if (playerPowerupHeld == Powerup::LIGHTNING) {
				std::shared_ptr<R_Scene> lightningStrike = resource->GetResource<R_Scene>(lightningPrefab);

				if (lightningStrike) {
					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					ecs::EntityID lightningStrikeID = DuplicatePrefabIntoScene<R_Scene>(currentScene, lightningPrefab);

					if (auto* lightningStrikeTransform = ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)) {
						RaycastHit hit;
						hit.entityID = 9999999;
						glm::vec3 dir = GetPlayerCameraFrontDirection();

						float range = ecsPtr->GetComponent<LightningPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)->m_childID[0])->range;
						physicsPtr->Raycast(cameraTransform->WorldTransformation.position, dir, range, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);

						if (hit.entityID != 9999999) {
							lightningStrikeTransform->LocalTransformation.position = hit.point;

							playerPowerupHeld = Powerup::NONE;
						}

					}
				}
			}
			else if (playerPowerupHeld == Powerup::FIREACID) {
				std::shared_ptr<R_Scene> flamethrower = resource->GetResource<R_Scene>(fireAcidPrefab);

				if (flamethrower) {
					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					ecs::EntityID flamethrowerID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireAcidPrefab);

					if (auto* flamethrowerTransform = ecsPtr->GetComponent<TransformComponent>(flamethrowerID)) {
						//flamethrowerTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
						ecsPtr->SetParent(playerProjectilePointObjectID, flamethrowerID);
						flamethrowerTransform->LocalTransformation.position = glm::vec3(0.f, 0.f, 0.f);

						// TODO: SOMEHOW MAKE THE FLAMETHROWER ABIDE TO THE PLAYER'S ROTATION
						//flamethrowerTransform->LocalTransformation.rotation = glm::vec3(playerRotationX, playerRotationY, 0.f);
					}
				}

				playerPowerupHeld = Powerup::NONE;
			}
			else if (playerPowerupHeld == Powerup::FIRELIGHTNING) {
				std::shared_ptr<R_Scene> groundSpikes = resource->GetResource<R_Scene>(fireLightningPrefab);

				if (groundSpikes) {
					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					ecs::EntityID groundSpikesID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireLightningPrefab);

					if (auto* groundSpikesTransform = ecsPtr->GetComponent<TransformComponent>(groundSpikesID)) {
						groundSpikesTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(entity)->WorldTransformation.position;
					}

					playerPowerupHeld = Powerup::NONE;
				}
			}
			else if (playerPowerupHeld == Powerup::ACIDLIGHTNING) {
				std::shared_ptr<R_Scene> starfall = resource->GetResource<R_Scene>(acidLightningPrefab);

				if (starfall) {
					// Update later
					//std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					//ecs::EntityID starfallID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidLightningPrefab);

					//if (auto* starfallTransform = ecsPtr->GetComponent<TransformComponent>(starfallID)) {
					//	starfallTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
					//}

					//glm::vec3 dir = GetPlayerCameraFrontDirection();
					//for (int i = 0; i < ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID.size(); ++i) {
					//	// TODO: RANDOMIZE THE DIRECTION OF THE STARFALL PROJECTILE HERE OR SOMETHING
					//	ecsPtr->GetComponent<LightningAcidPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID[i])->direction = dir;
					//}

					//playerPowerupHeld = Powerup::NONE;

					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					ecs::EntityID fireballID = DuplicatePrefabIntoScene<R_Scene>(currentScene, firePrefab);

					if (auto* fireballTransform = ecsPtr->GetComponent<TransformComponent>(fireballID)) {
						fireballTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(playerProjectilePointObjectID)->WorldTransformation.position;
					}

					if (auto* fireballScript = ecsPtr->GetComponent<FirePowerupManagerScript>(fireballID)) {
						fireballScript->direction = GetPlayerCameraFrontDirection();
					}

					playerPowerupHeld = Powerup::NONE;
				}
			}
		}

		// INTERACT
		if (Input->IsKeyTriggered(keys::RMB)) {

			bool hasAbsorbed = false;


			RaycastHit hit;
			hit.entityID = 9999999;
			physicsPtr->Raycast(cameraTransform->WorldTransformation.position, GetPlayerCameraFrontDirection(), interactPowerupRange, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);

			if (hit.entityID != 9999999 && ecsPtr->GetComponent<NameComponent>(hit.entityID)->entityTag == "Powerup") {
				if (auto* powerupComp = ecsPtr->GetComponent<PowerupManagerScript>(hit.entityID)) {
					hasAbsorbed = true;
					if (playerPowerupHeld == Powerup::NONE) {
						if (powerupComp->powerupType == "FIRE") {
							playerPowerupHeld = Powerup::FIRE;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "ACID") {
							playerPowerupHeld = Powerup::ACID;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "LIGHTNING") {
							playerPowerupHeld = Powerup::LIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
					}
					else if (playerPowerupHeld == Powerup::FIRE) {
						if (powerupComp->powerupType == "FIRE") {
							playerPowerupHeld = Powerup::FIRE;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "ACID") {
							playerPowerupHeld = Powerup::FIREACID;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "LIGHTNING") {
							playerPowerupHeld = Powerup::FIRELIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
					}
					else if (playerPowerupHeld == Powerup::ACID) {
						if (powerupComp->powerupType == "FIRE") {
							playerPowerupHeld = Powerup::FIREACID;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "ACID") {
							playerPowerupHeld = Powerup::ACID;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "LIGHTNING") {
							playerPowerupHeld = Powerup::ACIDLIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
					}
					else if (playerPowerupHeld == Powerup::LIGHTNING) {
						if (powerupComp->powerupType == "FIRE") {
							playerPowerupHeld = Powerup::FIRELIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "ACID") {
							playerPowerupHeld = Powerup::ACIDLIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
						else if (powerupComp->powerupType == "LIGHTNING") {
							playerPowerupHeld = Powerup::LIGHTNING;

							// ADD PARTICLE EFFECT HERE
						}
					}

					// ADD SFX OF POWERUP PICKUP HERE
				}
			}
			if (anim && hasAbsorbed)
			{
				if (anim->m_currentStateID)
				{
					//static_cast<AnimState*>(anim->m_currentState)->SetTrigger("hasAbsorbed");
				}
			}
		}
	}

	bool GroundCheck() {
		if (auto* groundCheck = ecsPtr->GetComponent<GroundCheckScript>(playerGroundCheckObjectID)) {
			return groundCheck->groundCheck;
		}

		return false;
	}

	//void TakeDamage(int amount) {
	//	if (isDead) return;

	//	currPlayerHitPoints -= amount;
	//	if (currPlayerHitPoints < 0) currPlayerHitPoints = 0;

	//	std::cout << "[PLAYER] Took damage: " << amount
	//		<< " | Health = " << currPlayerHitPoints << std::endl;

	//	UpdateHealthUI();

	//	if (currPlayerHitPoints <= 0) {
	//		Die();
	//	}
	//}

	//void Die() {
	//	isDead = true;

	//	std::cout << "[PLAYER] Died!" << std::endl;

	//	// Disable movement
	//	currPlayerMovSpeed = 0.f;
	//	currPlayerJumpForce = 0.f;

	//	// Play death animation or show UI
	//	//if (currPlayerHitPoints <= 0 && !WinScreenScript::isWinScreenActive) {
	//	//	if (auto* loseScreen = ecsPtr->GetComponent<WinScreenScript>(winScreenCanvasID)) {
	//	//		loseScreen->ShowWinScreen();
	//	//		return;
	//	//	}
	//	//}

	//	if (currPlayerHitPoints <= 0 && !LoseScreenScript::isLoseScreenActive) {
	//		if (auto* loseScreen = ecsPtr->GetComponent<LoseScreenScript>(loseScreenCanvasID)) {
	//			loseScreen->ShowLoseScreen();
	//			return;
	//		}
	//	}


	//}


	void UpdateHealthUI() {
		if (auto* text = ecsPtr->GetComponent<TextComponent>(healthUIObjectID)) {
			text->text = "HP: " + std::to_string(currPlayerHitPoints) + "/" + std::to_string(maxPlayerHitPoints);
		}
	}



	// HELPER FUNCTIONS
	glm::vec3 GetPlayerCameraFrontDirection() {
		float yaw = glm::radians(playerRotationY);
		float pitch = glm::radians(playerRotationX);

		glm::vec3 dir;
		dir.x = std::cos(pitch) * std::cos(yaw);
		dir.y = std::sin(pitch);
		dir.z = std::cos(pitch) * std::sin(yaw);

		dir = glm::normalize(dir);

		return dir;
	}

	glm::vec3 GetPlayerCameraRightDirection() {
		float yaw = glm::radians(playerRotationY);

		glm::vec3 dir;
		dir.x = std::sin(yaw);
		dir.y = 0.f;
		dir.z = -std::sin(yaw);

		dir = glm::normalize(dir);

		return dir;
	}

	glm::vec3 GetPlayerCameraUpDirection() {
		float yaw = glm::radians(playerRotationY);
		float pitch = glm::radians(playerRotationX);

		glm::vec3 dir;
		dir.x = -std::cos(pitch) * std::cos(yaw);
		dir.y = std::cos(pitch);
		dir.z = -std::sin(pitch) * std::sin(yaw);

		dir = glm::normalize(dir);

		return dir;
	}

	glm::vec3 GetPlayerFrontDirection() {
		float cameraRotationY = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.rotation.y;
		glm::vec3 dir(std::sin(glm::radians(-cameraRotationY + 90.f)), 0.f, std::cos(glm::radians(-cameraRotationY + 90.f)));

		return dir;
	}

	glm::vec3 GetPlayerRightDirection() {
		float cameraRotationY = ecsPtr->GetComponent<TransformComponent>(playerCameraObjectID)->LocalTransformation.rotation.y;
		glm::vec3 dir(std::sin(glm::radians(-cameraRotationY)), 0.f, std::cos(glm::radians(-cameraRotationY)));

		return dir;
	}

	REFLECTABLE(PlayerManagerScript, playerCameraObject, playerGunCameraObject, playerProjectilePointObject, playerGunModelPointObject, playerArmModelObject, playerGroundCheckObject,
		bulletPrefab, firePrefab, acidPrefab, lightningPrefab, fireAcidPrefab, fireLightningPrefab, acidLightningPrefab,
		gunSfxGUID_1, gunSfxGUID_2, pauseMenuManagerObject, healthUIObject, loseScreenCanvasObject, winScreenCanvasObject);

};




//#pragma once
//#include "TemplateSC.h"
////#include "Inputs/Input.h"
//
//class PlayerManagerScript : public TemplateSC {
//public:
//	int playerHealth;
//	float playerMovementSpeed;
//	float playerCrouchingSpeed;
//	float playerJumpForce;
//
//	float playerCameraSpeedX;
//	float playerCameraSpeedY;
//
//	float interactPowerupRange = 5.f;
//
//	utility::GUID creationPoint;
//	ecs::EntityID creationPointID;
//	utility::GUID cameraObject;
//	ecs::EntityID cameraObjectID;
//	utility::GUID armModel;
//	ecs::EntityID armModelID;
//	utility::GUID groundCheck;
//	ecs::EntityID groundCheckID;
//
//	utility::GUID bulletPrefab;
//	utility::GUID fireballPrefab;
//	utility::GUID lightningStrikePrefab;
//	utility::GUID acidBlastPrefab;
//	utility::GUID groundSpikesPrefab;
//	utility::GUID flamethrowerPrefab;
//	utility::GUID starfallPrefab;
//
//	utility::GUID gunSfxGUID;
//
//	float rotationX = 0.f, rotationY = 0.f;
//	bool cursorIsHidden = false;
//	glm::vec3 cameraFacingDirection;
//
//	std::string currentPowerup = "none";
//
//	ecs::TransformComponent* cameraTransform;
//	bool isGrounded = false;
//
//	float cameraStandingHeight, cameraCrouchingHeight, cameraSlidingHeight;
//
//	bool playerIsRunning = false, playerIsSliding = false;
//	float playerCurrentMovementSpeed;
//
//	// JUST FOR TESTING, DELETE AFTERWARDS
//	float originalDrag;
//
//	void Start() override {
//		creationPointID = ecsPtr->GetEntityIDFromGUID(creationPoint);
//		cameraObjectID = ecsPtr->GetEntityIDFromGUID(cameraObject);
//		armModelID = ecsPtr->GetEntityIDFromGUID(armModel);
//		groundCheckID = ecsPtr->GetEntityIDFromGUID(groundCheck);
//
//		cameraTransform = ecsPtr->GetComponent<ecs::TransformComponent>(cameraObjectID);
//		cameraStandingHeight = cameraTransform->LocalTransformation.position.y;
//		cameraCrouchingHeight = cameraStandingHeight * 0.5f;
//		cameraSlidingHeight = cameraStandingHeight * 0.3f;
//
//		playerCurrentMovementSpeed = playerMovementSpeed;
//		originalDrag = ecsPtr->GetComponent<ecs::RigidbodyComponent>(entity)->drag;
//	}
//
//	void Update() override {
//		if (auto* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity)) {
//
//			isGrounded = ecsPtr->GetComponent<GroundCheckScript>(groundCheckID)->groundCheck;
//
//			glm::vec3 rotationInDegrees(tc->LocalTransformation.rotation);
//			glm::vec3 rotationInRad = glm::radians(rotationInDegrees);
//			glm::quat q = glm::quat(rotationInRad);
//
//			glm::vec3 forward = q * glm::vec3(0.f, 0.f, 1.f);
//			glm::vec3 right = q * glm::vec3(1.f, 0.f, 0.f);
//
//			// Movement Inputs
//			glm::vec3 moveForce = {0.f, 0.f, 0.f};
//
//			if (Input->IsKeyPressed(keys::W)) {
//				if (Input->IsKeyPressed(keys::LeftShift)) {
//					//tc->LocalTransformation.position += glm::normalize(forward) * playerCurrentMovementSpeed * ecsPtr->m_GetDeltaTime() * 1.75f;
//					//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, glm::normalize(forward) * playerCurrentMovementSpeed * 1.45f, ForceMode::VelocityChange);
//					moveForce += glm::normalize(forward) * playerCurrentMovementSpeed * 1.45f;
//					playerIsRunning = true;
//				}
//				else {
//					//tc->LocalTransformation.position += glm::normalize(forward) * playerCurrentMovementSpeed * ecsPtr->m_GetDeltaTime();
//					//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, glm::normalize(forward) * playerCurrentMovementSpeed, ForceMode::VelocityChange);
//					moveForce += glm::normalize(forward) * playerCurrentMovementSpeed;
//				}
//			}
//
//			if (Input->IsKeyReleased(keys::LeftShift)) {
//				playerIsRunning = false;
//			}
//
//			if (Input->IsKeyPressed(keys::S)) {
//				//tc->LocalTransformation.position -= glm::normalize(forward) * playerCurrentMovementSpeed * ecsPtr->m_GetDeltaTime();
//				//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, -glm::normalize(forward) * playerCurrentMovementSpeed, ForceMode::VelocityChange);
//				moveForce += -glm::normalize(forward) * playerCurrentMovementSpeed;
//			}
//
//			if (Input->IsKeyPressed(keys::D)) {
//				//tc->LocalTransformation.position -= glm::normalize(right) * playerCurrentMovementSpeed * ecsPtr->m_GetDeltaTime();
//				//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, -glm::normalize(right) * playerCurrentMovementSpeed, ForceMode::VelocityChange);
//				moveForce += -glm::normalize(right) * playerCurrentMovementSpeed;
//			}
//
//			if (Input->IsKeyPressed(keys::A)) {
//				//tc->LocalTransformation.position += glm::normalize(right) * playerCurrentMovementSpeed * ecsPtr->m_GetDeltaTime();
//				//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, glm::normalize(right) * playerCurrentMovementSpeed, ForceMode::VelocityChange);
//				moveForce += glm::normalize(right) * playerCurrentMovementSpeed;
//			}
//
//			if (Input->IsKeyTriggered(keys::SPACE) && isGrounded) {
//				std::cout << "JUMPING\n";
//				physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, { 0.f, playerJumpForce, 0.f }, ForceMode::VelocityChange);
//			}
//
//			if (Input->IsKeyPressed(keys::LeftControl) && isGrounded) {
//				if (playerIsRunning) {
//					cameraTransform->LocalTransformation.position.y = cameraSlidingHeight;
//				}
//				else {
//					cameraTransform->LocalTransformation.position.y = cameraCrouchingHeight;
//					playerCurrentMovementSpeed = playerCrouchingSpeed;
//				}
//			}
//			else if (Input->IsKeyReleased(keys::LeftControl)) {
//				cameraTransform->LocalTransformation.position.y = cameraStandingHeight;
//				playerCurrentMovementSpeed = playerMovementSpeed;
//			}
//
//			physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, moveForce * ecsPtr->m_GetDeltaTime(), ForceMode::VelocityChange);
//			
//			if (!isGrounded) {
//				//physicsPtr->AddForce(ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor, moveForce * ecsPtr->m_GetDeltaTime(), ForceMode::VelocityChange);
//				ecsPtr->GetComponent<RigidbodyComponent>(entity)->drag = 0.f;
//				playerCurrentMovementSpeed = playerMovementSpeed * 0.05f;
//			}
//			else {
//				ecsPtr->GetComponent<RigidbodyComponent>(entity)->drag = originalDrag;
//				playerCurrentMovementSpeed = playerMovementSpeed;
//			}
//
//			// First Person Camera
//			// THIS IS SUPER CURSED FOR NOW I SWEAR -> HARDCODING THE CAMERA TO BE FIRST CHILD
//			//if (auto* cc = ecsPtr->GetComponent<ecs::CameraComponent>(ecsPtr->GetComponent<ecs::TransformComponent>(entity)->m_childID[0])) {	
//			if (auto* cc = ecsPtr->GetComponent<ecs::CameraComponent>(cameraObjectID)) {
//				//std::cout << "CAMERA EXISTS\n";
//				
//				float mouseRotationX = Input->GetAxisRaw("Mouse Y") * playerCameraSpeedX;
//				float mouseRotationY = Input->GetAxisRaw("Mouse X") * playerCameraSpeedY;
//				rotationX += mouseRotationX;
//				rotationY += mouseRotationY;
//				rotationX = glm::clamp(rotationX, -90.f, 90.f);
//				//auto* cameraTransform = ecsPtr->GetComponent<ecs::TransformComponent>(cameraObjectID);
//				cameraTransform->LocalTransformation.rotation = glm::vec3(rotationX, rotationY + 90.f, 0.f);
//				//std::cout << "CAMERA: " << cameraTransform->LocalTransformation.rotation.x << ", " << cameraTransform->LocalTransformation.rotation.y << std::endl;
//				tc->LocalTransformation.rotation = glm::vec3(0.f, -rotationY, 0.f);
//
//				if (auto* armModelTrans = ecsPtr->GetComponent<TransformComponent>(armModelID)) {
//					armModelTrans->LocalTransformation.rotation.x = -rotationX;
//				}
//
//				// Interact Inputs
//				if (Input->IsKeyTriggered(keys::E)) {
//					//std::cout << "Pressing E\n";
//					RaycastHit hit;
//					hit.entityID = 9999999;
//
//					//float yaw = glm::radians(rotationY + 90.f);
//					//float pitch = glm::radians(rotationX);
//
//					//glm::vec3 dir;
//					//dir.x = std::cos(pitch) * std::cos(yaw);
//					//dir.y = std::sin(pitch);
//					//dir.z = cos(pitch) * std::sin(yaw);
//
//					//dir = glm::normalize(dir);
//
//					glm::vec3 dir = GetCameraFacingDirection();
//					
//					physicsPtr->Raycast(cameraTransform->WorldTransformation.position, dir, 5.f, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);
//					//ecs::EntityID test = ecsPtr->CreateEntity(ecsPtr->GetSceneByEntityID(entity));
//					//ecsPtr->GetComponent<NameComponent>(test)->entityName = "Test";
//					//ecsPtr->AddComponent<BoxColliderComponent>(test);
//					//ecsPtr->GetComponent<BoxColliderComponent>(test)->isTrigger = true;
//					//ecsPtr->GetComponent<TransformComponent>(test)->LocalTransformation.position = cameraTransform->WorldTransformation.position + dir * 5.f;
//
//					if (hit.entityID != 9999999 && ecsPtr->GetComponent<NameComponent>(hit.entityID)->entityTag == "Powerup") {
//
//						if (auto* powerupComp = ecsPtr->GetComponent<PowerupManagerScript>(hit.entityID)) {
//							if (powerupComp->powerupType == "fire") {
//								if (currentPowerup == "lightning") {
//									currentPowerup = "firelightning";
//								}
//								else if (currentPowerup == "acid") {
//									currentPowerup = "fireacid";
//								}
//								else if(currentPowerup == "none") {
//									currentPowerup = "fire";
//								}
//							}
//							else if (powerupComp->powerupType == "lightning") {
//								if (currentPowerup == "fire") {
//									currentPowerup = "firelightning";
//								}
//								else if (currentPowerup == "acid") {
//									currentPowerup = "lightningacid";
//								}
//								else if (currentPowerup == "none") {
//									currentPowerup = "lightning";
//								}
//							}
//							else if (powerupComp->powerupType == "acid") {
//								if (currentPowerup == "fire") {
//									currentPowerup = "fireacid";
//								}
//								else if (currentPowerup == "lightning") {
//									currentPowerup = "lightningacid";
//								}
//								else if (currentPowerup == "none") {
//									currentPowerup = "acid";
//								}
//							}
//
//							std::cout << "CURRENT POWERUP: " << currentPowerup << std::endl;
//						}
//					}
//				}
//			}
//
//			// Shooting Inputs
//			if (Input->IsKeyTriggered(keys::LMB)) {
//				std::shared_ptr<R_Scene> bullet = resource->GetResource<R_Scene>(bulletPrefab);
//
//				if (bullet) {
//
//					if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
//						for (auto& af : ac->audioFiles) {
//							if (af.audioGUID == gunSfxGUID && af.isSFX) {
//								af.requestPlay = true;
//								break;
//							}
//						}
//					}
//
//					std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//					//ecs::EntityID bulletID = bullet->DuplicatePrefabIntoScene(currentScene);
//					ecs::EntityID bulletID = DuplicatePrefabIntoScene<R_Scene>(currentScene, bulletPrefab);
//
//					if (auto* bulletTransform = ecsPtr->GetComponent<TransformComponent>(bulletID)) {
//						bulletTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(creationPointID)->WorldTransformation.position;
//					}
//
//					if (auto* bulletScript = ecsPtr->GetComponent<BulletLogic>(bulletID)) {
//						bulletScript->direction = GetCameraFacingDirection();
//					}
//				}
//			}
//
//			// Powerup shooting
//			if (Input->IsKeyTriggered(keys::RMB)) {
//				if (currentPowerup == "fire") {
//					std::shared_ptr<R_Scene> fireball = resource->GetResource<R_Scene>(fireballPrefab);
//
//					if (fireball) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID fireballID = fireball->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID fireballID = DuplicatePrefabIntoScene<R_Scene>(currentScene, fireballPrefab);
//
//						if (auto* fireballTransform = ecsPtr->GetComponent<TransformComponent>(fireballID)) {
//							fireballTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(creationPointID)->WorldTransformation.position;
//						}
//
//						if (auto* fireballScript = ecsPtr->GetComponent<FirePowerupManagerScript>(fireballID)) {
//							fireballScript->direction = GetCameraFacingDirection();
//						}
//
//						currentPowerup = "none";
//					}
//				}
//				else if (currentPowerup == "lightning") {
//					std::shared_ptr<R_Scene> lightningStrike = resource->GetResource<R_Scene>(lightningStrikePrefab);
//
//					if (lightningStrike) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID lightningStrikeID = lightningStrike->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID lightningStrikeID = DuplicatePrefabIntoScene<R_Scene>(currentScene, lightningStrikePrefab);
//
//						if (auto* lightningStrikeTransform = ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)) {
//							RaycastHit hit;
//							hit.entityID = 9999999;
//							glm::vec3 dir = GetCameraFacingDirection();
//
//							float range = ecsPtr->GetComponent<LightningPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(lightningStrikeID)->m_childID[0])->range;
//							physicsPtr->Raycast(cameraTransform->WorldTransformation.position, dir, range, hit, ecsPtr->GetComponent<RigidbodyComponent>(entity)->actor);
//
//							if (hit.entityID != 9999999) {
//								lightningStrikeTransform->LocalTransformation.position = hit.point;
//
//								currentPowerup = "none";
//							}
//
//						}
//					}
//				}
//				else if (currentPowerup == "acid") {
//					std::shared_ptr<R_Scene> acidBlast = resource->GetResource<R_Scene>(acidBlastPrefab);
//
//					if (acidBlast) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID acidBlastID = acidBlast->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID acidBlastID = DuplicatePrefabIntoScene<R_Scene>(currentScene, acidBlastPrefab);
//
//						if (auto* acidBlastTransform = ecsPtr->GetComponent<TransformComponent>(acidBlastID)) {
//							acidBlastTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(creationPointID)->WorldTransformation.position;
//						}
//
//						if (auto* acidBlastScript = ecsPtr->GetComponent<AcidPowerupManagerScript>(acidBlastID)) {
//							acidBlastScript->direction = GetCameraFacingDirection();
//						}
//
//						currentPowerup = "none";
//					}
//				}
//				else if (currentPowerup == "firelightning") {
//					std::shared_ptr<R_Scene> groundSpikes = resource->GetResource<R_Scene>(groundSpikesPrefab);
//
//					if (groundSpikes) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID groundSpikesID = groundSpikes->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID groundSpikesID = DuplicatePrefabIntoScene<R_Scene>(currentScene, groundSpikesPrefab);
//
//						if (auto* groundSpikesTransform = ecsPtr->GetComponent<TransformComponent>(groundSpikesID)) {
//							groundSpikesTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(entity)->WorldTransformation.position;
//						}
//
//						currentPowerup = "none";
//					}
//				}
//				else if (currentPowerup == "fireacid") {
//					std::shared_ptr<R_Scene> flamethrower = resource->GetResource<R_Scene>(flamethrowerPrefab);
//
//					if (flamethrower) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID flamethrowerID = flamethrower->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID flamethrowerID = DuplicatePrefabIntoScene<R_Scene>(currentScene, flamethrowerPrefab);
//
//						if (auto* flamethrowerTransform = ecsPtr->GetComponent<TransformComponent>(flamethrowerID)) {
//							flamethrowerTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(creationPointID)->WorldTransformation.position;
//							//flamethrowerTransform.
//							ecsPtr->SetParent(creationPointID, flamethrowerID);
//						}
//
//						
//					}
//
//					currentPowerup = "none";
//				}
//				else if (currentPowerup == "lightningacid") {
//					std::shared_ptr<R_Scene> starfall = resource->GetResource<R_Scene>(starfallPrefab);
//
//					if (starfall) {
//						std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
//						//ecs::EntityID starfallID = starfall->DuplicatePrefabIntoScene(currentScene);
//						ecs::EntityID starfallID = DuplicatePrefabIntoScene<R_Scene>(currentScene, starfallPrefab);
//
//						if (auto* starfallTransform = ecsPtr->GetComponent<TransformComponent>(starfallID)) {
//							starfallTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(creationPointID)->WorldTransformation.position;
//						}
//
//						glm::vec3 dir = GetCameraFacingDirection();
//						for (int i = 0; i < ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID.size(); ++i) {
//							ecsPtr->GetComponent<LightningAcidPowerupManagerScript>(ecsPtr->GetComponent<TransformComponent>(starfallID)->m_childID[i])->direction = dir;
//						}
//
//						currentPowerup = "none";
//					}
//				}
//			}
//
//			// Hide Cursor
//			if (Input->IsKeyTriggered(keys::X)) {
//				if (cursorIsHidden) {
//					Input->HideCursor(false);
//					cursorIsHidden = false;
//				}
//				else {
//					Input->HideCursor(true);
//					cursorIsHidden = true;
//				}
//			}
//
//			// Spawn another enemy
//			if (Input->IsKeyTriggered(keys::Z)) {
//
//			}
//		}
//	}
//
//	glm::vec3 GetCameraFacingDirection() {
//		float yaw = glm::radians(rotationY + 90.f);
//		float pitch = glm::radians(rotationX);
//
//		glm::vec3 dir;
//		dir.x = std::cos(pitch) * std::cos(yaw);
//		dir.y = std::sin(pitch);
//		dir.z = cos(pitch) * std::sin(yaw);
//
//		dir = glm::normalize(dir);
//
//		return dir;
//	}
//
//	REFLECTABLE(PlayerManagerScript, playerHealth, playerMovementSpeed, playerCrouchingSpeed, playerJumpForce, playerCameraSpeedX,
//		playerCameraSpeedY, creationPoint, cameraObject, armModel, groundCheck, bulletPrefab, fireballPrefab, lightningStrikePrefab,
//		acidBlastPrefab, groundSpikesPrefab, starfallPrefab, gunSfxGUID);
//};