
#include "Config/pch.h"
#include "ECS/ECS.h"
#include "Scene/SceneManager.h"
#include "Scripts/Include/ScriptHeader.h"
#include "Inputs/Input.h"
#include "Physics/PhysicsManager.h"
#include "Scripting/ScriptManager.h"
#include "Pathfinding/NavMesh.h"

static std::vector<std::string>* scriptNames;

template <typename T>
void RegisterScript(ecs::ECS* ecs) {
	FieldComponentTypeRegistry::RegisterComponentType<T>();
	ecs->RegisterComponent<T>();
	scriptNames->emplace_back(T::classname());
}

extern "C"  __declspec(dllexport) void UpdateStatic(StaticVariableManager* svm) {
	//ScriptManager::m_GetInstance();
	//ecs::ECS::m_in
	//Link the ECS system together
	//TemplateSC::ecsPtr = std::shared_ptr<ecs::ECS>(reinterpret_cast<ecs::ECS*>(svm->ECSSystem), [](ecs::ECS*) { /* no delete, managed elsewhere */ }).get();
	scriptNames = svm->scriptNames;


	TemplateSC::ecsPtr = static_cast<ecs::ECS*>(svm->ECSSystem);
	TemplateSC::Input = static_cast<Input::InputSystem*>(svm->input);
	TemplateSC::Scenes = static_cast<scenes::SceneManager*>(svm->scene);
	TemplateSC::physicsPtr = static_cast<physics::PhysicsManager*>(svm->physics);
	TemplateSC::resource = static_cast<ResourceManager*>(svm->resource);
	TemplateSC::navMeshPtr = static_cast<NavMeshManager*>(svm->navMesh);

	RegisterScript<BulletLogic>(TemplateSC::ecsPtr);
	RegisterScript<FireLMB>(TemplateSC::ecsPtr);
	RegisterScript<AcidLMB>(TemplateSC::ecsPtr);
	RegisterScript<LightningLMB>(TemplateSC::ecsPtr);
	RegisterScript<EnemyManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<PowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<FireballSplash>(TemplateSC::ecsPtr);
	RegisterScript<AcidAirBlast>(TemplateSC::ecsPtr);

	RegisterScript<FirePowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<LightningPowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<AcidPowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<AcidShield>(TemplateSC::ecsPtr);
	RegisterScript<AcidGas>(TemplateSC::ecsPtr);

	RegisterScript<FireLightningPowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<FireAcidPowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<LightningAcidPowerupManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<FireDashVfxScript>(TemplateSC::ecsPtr);

	RegisterScript<GroundCheckScript>(TemplateSC::ecsPtr);
	RegisterScript<PlayerManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<LevelCompleteScript>(TemplateSC::ecsPtr);

	RegisterScript<GameManager>(TemplateSC::ecsPtr);

	RegisterScript<EnemyHurtboxScript>(TemplateSC::ecsPtr);
	RegisterScript<EnemyBulletLogic>(TemplateSC::ecsPtr);
	RegisterScript<TankAOEScript>(TemplateSC::ecsPtr);

	RegisterScript<PlayerScript>(TemplateSC::ecsPtr);
	RegisterScript<EnemyScripts>(TemplateSC::ecsPtr);
	RegisterScript<AudioScript>(TemplateSC::ecsPtr);

	RegisterScript<EventColliderScript>(TemplateSC::ecsPtr);

	RegisterScript<EnemyHealthScript>(TemplateSC::ecsPtr);
	RegisterScript<AbilityUIImageScript>(TemplateSC::ecsPtr);
	RegisterScript<ScoreManagerScript>(TemplateSC::ecsPtr);
	RegisterScript<PauseMenuScript>(TemplateSC::ecsPtr);
	RegisterScript<LoseScreenScript>(TemplateSC::ecsPtr);
	RegisterScript<WinScreenScript>(TemplateSC::ecsPtr);

	RegisterScript<AnimationTestScript>(TemplateSC::ecsPtr);
	RegisterScript<UIButtonScript>(TemplateSC::ecsPtr);
	RegisterScript<NEWHealthBarScript>(TemplateSC::ecsPtr);
	RegisterScript<CutSceneScript>(TemplateSC::ecsPtr);
	RegisterScript<ManaUIScript>(TemplateSC::ecsPtr);
	RegisterScript<AmmoUIScript>(TemplateSC::ecsPtr);
	RegisterScript<NextSceneColliderScript>(TemplateSC::ecsPtr);
	RegisterScript<Ability1Script>(TemplateSC::ecsPtr);

	RegisterScript<RoomLockScript>(TemplateSC::ecsPtr);

	FieldComponentTypeRegistry::CreateAllDrawers(static_cast<Fields*>(svm->field)->GetAction());
}