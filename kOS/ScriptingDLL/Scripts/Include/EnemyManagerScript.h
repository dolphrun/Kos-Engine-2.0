#pragma once
#include "ScriptAdapter/TemplateSC.h"

// Forward Declaration (Crucial for compiling)
class EnemyBulletLogic;
class TankAOEScript;

class EnemyManagerScript : public TemplateSC {
public:
	R_AnimController* enemyController = nullptr;
	AnimatorComponent* animComp = nullptr;
	AnimState currAnimationState{};

	// Set dis  to "Melee" or "Ranged" or "Tank" in the editor.
	std::string enemyType = "Melee";

	int agentid;
	int enemyHealth;
	int maxEnemyHealth = 0;
	int lightningStack = 0;
	float enemyMovementSpeed;

	bool enemyIsAttacking = false;

	bool isStaggered = false;
	float currentStaggerTimer = 0.f;
	glm::vec3 pushbackVelocity = glm::vec3(0.f);

	bool isDead = false;
	float currentDeathTimer = 2.0f;

	float timeBeforeDamageByFlamethrowerAgain = 0.f;

	// --- TANK SHIELD VARIABLES ---
	int shieldHealth = 30;
	std::string shieldElement = "NONE"; // Set to "FIRE", "ACID", or "LIGHTNING" in Editor
	utility::GUID shieldVisualObject;   // GUID of the box indicating the shield
	ecs::EntityID shieldVisualID;   // The spawned visual entity ID

	// Separate variables for ranges cuz if not the ranged will kiss u
	float enemyAttackRange = 8.f;
	float enemyRangedAttackRange = 15.f;

	float enemyChaseRange = 25.f;

	utility::GUID playerToChase;
	ecs::EntityID playerToChaseID;

	utility::GUID enemyHurtboxPrefab;
	utility::GUID enemyBulletPrefab;
	utility::GUID tankAoePrefab;

	// --- LUNGE VARIABLES (it's gotta be fake lunging, not physics due to isKinematic) ---
	bool isLunging = false;
	glm::vec3 lungeVelocity = glm::vec3(0.f);
	float currentLungeTimer = 0.f;

	float lungeDuration = 0.8f;
	float lungeForwardSpeed = 12.f;
	float lungeUpwardSpeed = 5.f;
	float lungeGravity = 15.f;       // Fake gravity pulling them down

	float attackCooldown = 2.0f;
	float currentAttackCooldown = 0.f;

	utility::GUID enemyHurtboxPosition;
	ecs::EntityID enemyHurtboxPositionID;

	ecs::EntityID enemyModelID;

	bool attackHurtboxIsSpawn = false;

	utility::GUID enemyHurtVFXPrefab;
	utility::GUID enemyHurtVFXPosition;
	ecs::EntityID enemyHurtVFXPositionID;

	utility::GUID enemyAttackSfxGUID;
	utility::GUID enemyWalkSfxGUID;
	std::vector<utility::GUID> enemyHurtSfxPool;

	// Declarations Only
	void Start() override;
	void Update() override;
	void TriggerStagger(float duration);
	void ApplyPushback(glm::vec3 dir, float force);
	void TakeDamage(int damage, const std::string& element);
	void Die();

	REFLECTABLE(EnemyManagerScript, enemyHealth, enemyMovementSpeed, enemyType, enemyAttackRange, enemyRangedAttackRange, enemyChaseRange, playerToChase, 
		enemyHurtboxPrefab, enemyBulletPrefab, enemyHurtboxPosition, shieldHealth, shieldElement, shieldVisualObject, tankAoePrefab, isLunging, lungeDuration,
		lungeForwardSpeed, lungeUpwardSpeed, lungeGravity, attackCooldown, enemyHurtVFXPrefab, enemyHurtVFXPosition, enemyAttackSfxGUID,enemyWalkSfxGUID, enemyHurtSfxPool);
};

// --- IMPLEMENTATION ---
#include "EnemyBulletLogic.h"
#include "TankAOEScript.h"

inline void EnemyManagerScript::Start() {
	maxEnemyHealth = enemyHealth;
	playerToChaseID = ecsPtr->GetEntityIDFromGUID(playerToChase);
	//enemyHurtboxPositionID = ecsPtr->GetEntityIDFromGUID(enemyHurtboxPosition);

	auto* trans = ecsPtr->GetComponent<TransformComponent>(entity);
	auto* capsule = ecsPtr->GetComponent<CapsuleColliderComponent>(entity);
	navMeshPtr->AddAgent(agentid, entity, trans->WorldTransformation.position, capsule->capsule.radius, capsule->capsule.height);

	std::vector<EntityID> children = ecsPtr->GetChild(entity).value();
	if (children.size() > 1)
	{
		enemyModelID = children[1];
		enemyHurtboxPositionID = children[0];
	}

	// TANK: see if need change depending on how the designers want the shield
	if (enemyType == "Tank" && shieldElement != "NONE") {
		if (children.size() > 2) {
			shieldVisualID = children[2];
		}
	}

	enemyHurtVFXPositionID = ecsPtr->GetEntityIDFromGUID(enemyHurtVFXPosition);

	//enemyModelID = ecsPtr->GetEntityIDFromGUID(enemyModel);
	if (animComp = ecsPtr->GetComponent<ecs::AnimatorComponent>(enemyModelID))
	{
		enemyController = resource->GetResource<R_AnimController>(animComp->controllerGUID).get();
		if (enemyController)
		{
			animComp->m_currentStateID = enemyController->m_EnterState->id;
			if (auto* currAnimState = enemyController->RetrieveStateByID(animComp->m_currentStateID))
				currAnimState->Trigger("ForcedEntry", animComp, enemyController);
		}
	}
}

inline void EnemyManagerScript::Update() {
	auto* enemyTransform = ecsPtr->GetComponent<TransformComponent>(entity);
	auto* playerTransform = ecsPtr->GetComponent<TransformComponent>(playerToChaseID);
	auto* enemyHurtboxPositionTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxPositionID);

	if (!playerTransform || !enemyTransform || !enemyHurtboxPositionTransform) {
		LOGGING_WARN("Components not found");
		return;
	}

	

	//Entity deletion fix for animation
	animComp = ecsPtr->GetComponent<ecs::AnimatorComponent>(enemyModelID);

	if (isDead) {
		// UNCOMMENT THIS WHEN DEATH ANIMATION IS READY AND REMOVE THE TIMER BELOW
		/*
		if (animComp) {
			if (animComp->m_currentStateID) {
				if (R_Animation* currAnim = resource->GetResource<R_Animation>(enemyController->RetrieveStateByID(animComp->m_currentStateID)->animationGUID).get()) {
					float animDuration = currAnim->GetDuration();
					if (animComp->m_CurrentTime >= anim
) {
						ecsPtr->DeleteEntity(entity);
					}
				}
			}
		} else {
			ecsPtr->DeleteEntity(entity);
		}
		return;
		*/

		// TEMPORARY TIMER DEATH LOGIC
		currentDeathTimer -= ecsPtr->m_GetDeltaTime();
		if (currentDeathTimer <= 0.0f) {
			ecsPtr->DeleteEntity(entity);
		}
		return;
	}

	if (isStaggered) {
		enemyTransform->LocalTransformation.position += pushbackVelocity * ecsPtr->m_GetDeltaTime();
		pushbackVelocity = glm::mix(pushbackVelocity, glm::vec3(0.f), 10.f * ecsPtr->m_GetDeltaTime());

		currentStaggerTimer -= ecsPtr->m_GetDeltaTime();
		if (currentStaggerTimer <= 0.0f) {
			isStaggered = false;
			auto* capsule = ecsPtr->GetComponent<CapsuleColliderComponent>(entity);
			navMeshPtr->AddAgent(agentid, entity, enemyTransform->WorldTransformation.position, capsule->capsule.radius, capsule->capsule.height);
		}
	}

	// Cooldown for lunging
	if (currentAttackCooldown > 0.f) {
		currentAttackCooldown -= ecsPtr->m_GetDeltaTime();
	}

	// Lunging logic
	if (isLunging && !isDead) {
		// Manual move (have to bypass Rigidbody for dis)
		enemyTransform->LocalTransformation.position += lungeVelocity * ecsPtr->m_GetDeltaTime();

		// Grav
		lungeVelocity.y -= lungeGravity * ecsPtr->m_GetDeltaTime();

		currentLungeTimer -= ecsPtr->m_GetDeltaTime();

		// Landing cond (based on time, kinda eah but wtv)
		if (currentLungeTimer <= 0.0f) {
			isLunging = false;

			// Re-add to NavMesh where they landed
			auto* capsule = ecsPtr->GetComponent<CapsuleColliderComponent>(entity);
			if (capsule) {
				navMeshPtr->AddAgent(agentid, entity, enemyTransform->WorldTransformation.position, capsule->capsule.radius, capsule->capsule.height);
			}

			// --- LANDING ANIMATION TRIGGER ---
			if (animComp && animComp->m_currentStateID) {
				enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("Land", animComp, enemyController);
			}

			enemyIsAttacking = false;
			attackHurtboxIsSpawn = false;
		}
	}

	// FUCK
	enemyHurtboxPositionTransform->LocalTransformation.position.z = 1.f;

	// CONSTANTLY LOOK AT PLAYER
	glm::vec3 direction = playerTransform->LocalTransformation.position - enemyTransform->LocalTransformation.position;
	if (glm::length(direction) > 0.001f) direction = glm::normalize(direction);

	if (!isStaggered) {
		// Calculate yaw (rotation around Y axis)
		float yaw = std::atan2(direction.x, direction.z);

		// Calculate pitch (rotation around X axis)
		float pitch = std::asin(-direction.y);

		// Roll is typically 0 for forward-facing directions
		float roll = 0.0f;

		glm::vec3 rotation(0.f, yaw, 0.f);
		glm::vec3 rotationDegrees = glm::degrees(rotation);

		enemyTransform->LocalTransformation.rotation = rotationDegrees;
	}

	// COMMENTED OUT FOR ANIM


	if (animComp)
	{
		if (R_Animation* currAnim = resource->GetResource<R_Animation>(enemyController->RetrieveStateByID(animComp->m_currentStateID)->animationGUID).get())
		{
			float animDuration = currAnim->GetDuration();
			//Checking if animation is done
			if (animComp->m_CurrentTime >= animDuration && !enemyController->RetrieveStateByID(animComp->m_currentStateID)->isLooping)
			{
				// Only auto-finish for Ranged/Tank attacks now.
				// Melee will finish when it explicitly lands
				if (!isLunging) {
					enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("AnimationFinished", animComp, enemyController);
					enemyIsAttacking = false;
					attackHurtboxIsSpawn = false;
				}
			}
		}
	}

	if (!isStaggered) {
		// SWITCH DISTANCE BASED ON STRING
		float currentActiveRange = (enemyType == "Ranged") ? enemyRangedAttackRange : enemyAttackRange;

		if (glm::distance(enemyTransform->LocalTransformation.position, playerTransform->LocalTransformation.position) <= currentActiveRange && currentAttackCooldown <= 0.f) {

			// DONT UNCOMMENT THIS
			//if (!enemyIsAttacking) {
			//	std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

			//	if (enemyHurtbox) {
			//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			//		ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

			//		if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
			//			enemyHurtboxTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxPositionID)->WorldTransformation.position;
			//		}
			//	}
			//}
			//if (!enemyIsAttacking) {
			//	std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

			//	if (enemyHurtbox) {
			//		std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			//		ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

			//		if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
			//			enemyHurtboxTransform->LocalTransformation.position = enemyTransform->LocalTransformation.position + direction;
			//		}
			//	}
			//}
			enemyIsAttacking = true;
			if (animComp)
			{
				if (animComp->m_currentStateID)
				{
					enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("AttackingPlayer", animComp, enemyController);
				}

			}
		}

		if (enemyIsAttacking) {
			// NAVMESH STOP FOLLOWING
			// Not calling MoveAgent stops following - SF
			// ADD ENEMY ATTACKING ANIMATION

			// FORCE STOP for Ranged enemies
			if (enemyType == "Ranged") {
				navMeshPtr->MoveAgent(agentid, enemyTransform->WorldTransformation.position);
			}

			// BEFORE CODE WORKS, I TESTED
			// if (CHECK IF ANIMATION OF THE ENEMY IS AFTER THE ENEMY CLAWED OR SOME SHIT (e.g: ANIMATION TIMER IS AT 2s MARK)) {
					//std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

					//if (enemyHurtbox) {
					//	std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
					//	ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

					//	if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
					//		enemyHurtboxTransform->LocalTransformation.position = enemyTransform->LocalTransformation.position + direction;
					//	}
					//}
			// }


			// if (CHECK IF ANIMATION IS DONE) {
			//		enemyIsAttacking = false;
			// }
			if (animComp && !attackHurtboxIsSpawn)
			{
				// COMMENTED OUT FOR ANIM

				if (animComp->m_currentStateID)
				{
					R_Animation* currAnim = resource->GetResource<R_Animation>(enemyController->RetrieveStateByID(animComp->m_currentStateID)->animationGUID).get();
					float animDuration = currAnim->GetDuration();

					//CHECK IF ANIMATION OF THE ENEMY IS AFTER THE ENEMY CLAWED OR SOME SHIT(e.g: ANIMATION TIMER IS AT 2s MARK
					//Simulating 2 secconds, you might wanna change this
					if (animComp->m_CurrentTime >= animDuration * 0.5f)
					{
						std::string stateName = enemyController->RetrieveStateByID(animComp->m_currentStateID)->name;
						// SWITCH SPAWN BEHAVIOR BASED ON STRING
						if (enemyType == "Ranged" && stateName == "Attacking")
						{
							// Ranged: Spawn Bullet
							std::shared_ptr<R_Scene> bullet = resource->GetResource<R_Scene>(enemyBulletPrefab);

							if (bullet) {
								std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
								ecs::EntityID bulletID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyBulletPrefab);

								if (auto* bulletTransform = ecsPtr->GetComponent<TransformComponent>(bulletID)) {
									// CHEESE WAY TO LOWER SHOT HEIGHT FOR NOW THE PLAYER TOO SHORTO
									bulletTransform->LocalTransformation.position = enemyHurtboxPositionTransform->WorldTransformation.position - glm::vec3(0.0f, 1.0f, 0.0f);
								}

								if (auto* bulletScript = ecsPtr->GetComponent<EnemyBulletLogic>(bulletID)) {
									bulletScript->direction = direction;
								}
							}
						}
						else if (enemyType == "Tank" && stateName == "Attacking") {
							std::shared_ptr<R_Scene> tankAOE = resource->GetResource<R_Scene>(tankAoePrefab);
							if (tankAOE) {
								std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
								ecs::EntityID aoeID = DuplicatePrefabIntoScene<R_Scene>(currentScene, tankAoePrefab);

								// 1. PARENT THE AOE TO THE TANK! (false = don't keep world transform)
								ecsPtr->SetParent(entity, aoeID, false);

								if (auto* aoeTransform = ecsPtr->GetComponent<TransformComponent>(aoeID)) {
									// 2. Since it's a child now, 0,0,0 means it's perfectly centered on the Tank!
									aoeTransform->LocalTransformation.position = glm::vec3(0.f, 0.f, 0.f);
								}

								// Pass the Tank's ID so it doesn't hurt itself
								if (auto* aoeScript = ecsPtr->GetComponent<TankAOEScript>(aoeID)) {
									aoeScript->casterID = entity;
								}
							}
						}
						else if (stateName == "Crouching")
						{
							// Melee lunge logic
							isLunging = true;
							currentLungeTimer = lungeDuration;

							currentAttackCooldown = attackCooldown;

							navMeshPtr->RemoveAgent(agentid);

							glm::vec3 flatDir = glm::vec3(direction.x, 0.f, direction.z);
							if (glm::length(flatDir) > 0.001f) flatDir = glm::normalize(flatDir);

							lungeVelocity = flatDir * lungeForwardSpeed;

							lungeVelocity.y = lungeUpwardSpeed;

							// Height diff stuff inshallah
							float heightDiff = playerTransform->LocalTransformation.position.y - enemyTransform->LocalTransformation.position.y;
							if (heightDiff > 0.5f) {
								lungeVelocity.y += (heightDiff * 2.0f); // Tweak the float to make them jump higher if needed
							}

							// Hurtbox spawn
							std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);
							if (enemyHurtbox) {
								std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
								ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

								ecsPtr->SetParent(entity, enemyHurtboxID, false);

								if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
									// Push forward a bit
									enemyHurtboxTransform->LocalTransformation.position = glm::vec3(0.f, 0.5f, 1.f);
								}
							}

							// --- LUNGE ANIM TRIGGER ---
							if (animComp && animComp->m_currentStateID) {
								enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("Lunge", animComp, enemyController);
							}
						}

						attackHurtboxIsSpawn = true;
					}
				}

			}

		}
		else if (glm::distance(enemyTransform->LocalTransformation.position, playerTransform->LocalTransformation.position) <= enemyChaseRange) {
			// NAVMESH FOLLOW TOWARDS PLAYER
			navMeshPtr->MoveAgent(agentid, playerTransform->LocalTransformation.position);

			// ADD ENEMY RUNNING ANIMATION
			if (animComp)
			{
				if (animComp->m_currentStateID)
				{
					enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("PlayerDetected", animComp, enemyController);
				}
			}
		}
	}

	if (timeBeforeDamageByFlamethrowerAgain >= 0.f) {
		timeBeforeDamageByFlamethrowerAgain -= ecsPtr->m_GetDeltaTime();
	}
}

inline void EnemyManagerScript::TriggerStagger(float duration) {
	// Prevent engine crash: Don't remove NavMesh if already lunging!
	if (!isStaggered && !isLunging) {
		navMeshPtr->RemoveAgent(agentid);
	}

	isStaggered = true;
	isLunging = false; // Safely cancel the lunge mid-air
	currentStaggerTimer = duration;

	// ADD STAGGER ANIM HERE
	if (animComp)
	{
		if (animComp->m_currentStateID)
		{
			//enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("Staggered", animComp, enemyController);
			enemyController->PlayOverlay("Stagger", animComp, 0.2f, 0.8f);
		}
	}

	enemyIsAttacking = false;
	attackHurtboxIsSpawn = false;
}

inline void EnemyManagerScript::ApplyPushback(glm::vec3 dir, float force) {
	glm::vec3 flatDir = dir;
	flatDir.y = 0.f;
	if (glm::length(flatDir) > 0.001f) flatDir = glm::normalize(flatDir);
	pushbackVelocity = flatDir * force;
}

inline void EnemyManagerScript::TakeDamage(int damage, const std::string& element) {
	
	//Enemy Hurt sfx
	if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {
		std::vector<ecs::AudioFile*> hurtSfxMatches;

		for (auto& af : ac->audioFiles) {
			if (!af.isSFX) continue;

			for (const auto& poolGUID : enemyHurtSfxPool) {
				if (af.audioGUID == poolGUID) {
					hurtSfxMatches.push_back(&af);
					break;
				}
			}
		}

		//Random Play 
		if (!hurtSfxMatches.empty()) {
			int idx = rand() % static_cast<int>(hurtSfxMatches.size());
			hurtSfxMatches[idx]->requestPlay = true;
		}
	}


	if (shieldHealth > 0 && shieldElement != "NONE") {

		if (element == shieldElement) {
			shieldHealth -= damage;

			if (shieldHealth <= 0) {
				shieldHealth = 0;

				if (shieldVisualID != 0) {
					ecsPtr->DeleteEntity(shieldVisualID);
					shieldVisualID = 0;
				}
			}
		}
		else {
			// Its just immune to the power, this else is only here cuz it was for a cout, might remove in the future
		}
	}
	else {
		std::shared_ptr<R_Scene> enemyHurtVFX = resource->GetResource<R_Scene>(enemyHurtVFXPrefab);
		if (enemyHurtVFX) {
			std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
			ecs::EntityID enemyHurtVFXID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtVFXPrefab);

			if (auto* enemyHurtVFXTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtVFXID))
				enemyHurtVFXTransform->LocalTransformation.position = ecsPtr->GetComponent<TransformComponent>(enemyHurtVFXPositionID)->WorldTransformation.position;
		}

		// Normal health damage (Shield is gone or never existed)
		enemyHealth -= damage;

		if (enemyHealth < 0) {
			enemyHealth = 0;
		}
	}
}

inline void EnemyManagerScript::Die() {
	if (isDead) return;

	// ADD DEATH ANIM HERE
	if (animComp)
	{
		/*if (animComp->m_currentStateID)
		{
			enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("Dead", animComp, enemyController);
		}*/
		enemyController->SetState("Death", animComp);
	}

	

	isDead = true;

	if (!isStaggered) {
		navMeshPtr->RemoveAgent(agentid);
	}
}