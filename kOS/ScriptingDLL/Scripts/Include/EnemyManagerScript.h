#pragma once
#include "ScriptAdapter/TemplateSC.h"

// Forward Declaration (Crucial for compiling)
class EnemyBulletLogic;

class EnemyManagerScript : public TemplateSC {
public:
	R_AnimController* enemyController = nullptr;
	AnimatorComponent* animComp = nullptr;
	AnimState currAnimationState{};

	// Set dis  to "Melee" or "Ranged" in the editor.
	std::string enemyType = "Melee";

	int agentid;
	int enemyHealth;
	float enemyMovementSpeed;

	bool enemyIsAttacking = false;

	float timeBeforeDamageByFlamethrowerAgain = 0.f;

	// Separate variables for ranges cuz if not the ranged will kiss u
	float enemyAttackRange = 2.5f;
	float enemyRangedAttackRange = 2.5f;

	float enemyChaseRange = 25.f;

	utility::GUID playerToChase;
	ecs::EntityID playerToChaseID;

	utility::GUID enemyHurtboxPrefab;
	utility::GUID enemyBulletPrefab;

	utility::GUID enemyHurtboxPosition;
	ecs::EntityID enemyHurtboxPositionID;

	ecs::EntityID enemyModelID;

	bool attackHurtboxIsSpawn = false;

	// Declarations Only
	void Start() override;
	void Update() override;

	REFLECTABLE(EnemyManagerScript, enemyHealth, enemyMovementSpeed, enemyType, enemyAttackRange, enemyRangedAttackRange, enemyChaseRange, playerToChase, enemyHurtboxPrefab, enemyBulletPrefab, enemyHurtboxPosition);
};

// --- IMPLEMENTATION ---
#include "EnemyBulletLogic.h"

inline void EnemyManagerScript::Start() {
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

	//enemyModelID = ecsPtr->GetEntityIDFromGUID(enemyModel);
	if (animComp = ecsPtr->GetComponent<ecs::AnimatorComponent>(enemyModelID))
	{
		enemyController = resource->GetResource<R_AnimController>(animComp->controllerGUID).get();
		if (enemyController)
		{
			// COMMENTED OUT FOR ANIM
			/*currAnimationState = *enemyController->m_EnterState;
			anim->m_currentState = &currAnimationState;
			static_cast<AnimState*>(anim->m_currentState)->SetTrigger("ForcedEntry");*/
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

	//Copy state from controller
	if (enemyController)
	{
		//if (anim->m_currentState != &currAnimationState) {
		//	currAnimationState = *static_cast<AnimState*>(anim->m_currentState);
		//	anim->m_currentState = &currAnimationState;
		//}
	}

	// FUCK
	enemyHurtboxPositionTransform->LocalTransformation.position.z = 1.f;

	// CONSTANTLY LOOK AT PLAYER
	glm::vec3 direction = playerTransform->LocalTransformation.position - enemyTransform->LocalTransformation.position;
	direction = glm::normalize(direction);

	// Calculate yaw (rotation around Y axis)
	float yaw = std::atan2(direction.x, direction.z);

	// Calculate pitch (rotation around X axis)
	float pitch = std::asin(-direction.y);

	// Roll is typically 0 for forward-facing directions
	float roll = 0.0f;

	glm::vec3 rotation(0.f, yaw, 0.f);
	glm::vec3 rotationDegrees = glm::degrees(rotation);

	enemyTransform->LocalTransformation.rotation = rotationDegrees;

	// COMMENTED OUT FOR ANIM
	
	if (animComp)
	{
		R_Animation* currAnim = resource->GetResource<R_Animation>(enemyController->RetrieveStateByID(animComp->m_currentStateID)->animationGUID).get();
		float animDuration = currAnim->GetDuration();
		//Checkcing if animation is done
		if (animComp->m_CurrentTime >= animDuration && !enemyController->RetrieveStateByID(animComp->m_currentStateID)->isLooping)
		{
			enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("AnimationFinished", animComp, enemyController);
			//animComp->m_CurrentTime = 0.f;
			enemyIsAttacking = false;
			attackHurtboxIsSpawn = false;
		}
	}
	

	// SWITCH DISTANCE BASED ON STRING
	float currentActiveRange = (enemyType == "Ranged") ? enemyRangedAttackRange : enemyAttackRange;

	if (glm::distance(enemyTransform->LocalTransformation.position, playerTransform->LocalTransformation.position) <= currentActiveRange) {

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
			// COMMENTED OUT FOR ANIM
			/*
			if (anim->m_currentState)
			{
				//This will transition into attacking state
				static_cast<AnimState*>(anim->m_currentState)->SetTrigger("AttackingPlayer");
			}
			*/
			if (animComp->m_currentStateID)
			{
				enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("AttackingPlayer",animComp,enemyController);
			}
			//std::cout << "Ataccking" << std::endl;
			//std::cout << enemyController->RetrieveStateByID(animComp->m_currentStateID)->name << std::endl;
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
				if (animComp->m_CurrentTime >= animDuration * 0.5f && enemyController->RetrieveStateByID(animComp->m_currentStateID)->name == "Attacking")
				{
					// SWITCH SPAWN BEHAVIOR BASED ON STRING
					if (enemyType == "Ranged")
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
					else
					{
						// Default/Melee: Spawn Hurtbox
						// Used 'else' here so if you typo the string, it at least does something (Melee)
						std::shared_ptr<R_Scene> enemyHurtbox = resource->GetResource<R_Scene>(enemyHurtboxPrefab);

						if (enemyHurtbox) {
							std::string currentScene = ecsPtr->GetSceneByEntityID(entity);
							ecs::EntityID enemyHurtboxID = DuplicatePrefabIntoScene<R_Scene>(currentScene, enemyHurtboxPrefab);

							if (auto* enemyHurtboxTransform = ecsPtr->GetComponent<TransformComponent>(enemyHurtboxID)) {
								enemyHurtboxTransform->LocalTransformation.position = enemyTransform->LocalTransformation.position + direction;
							}
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
			// COMMENTED OUT FOR ANIM
			/*
			if (anim->m_currentState)
			{
				//This will transition into chasing state
				static_cast<AnimState*>(anim->m_currentState)->SetTrigger("PlayerDetected");
			}
			*/
			if (animComp->m_currentStateID)
				enemyController->RetrieveStateByID(animComp->m_currentStateID)->Trigger("PlayerDetected", animComp, enemyController);

		}
	}

	if (timeBeforeDamageByFlamethrowerAgain >= 0.f) {
		timeBeforeDamageByFlamethrowerAgain -= ecsPtr->m_GetDeltaTime();
	}
}