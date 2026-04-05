    #pragma once
    #include "ScriptAdapter/TemplateSC.h"

    // FORWARD DECLARATION
    class EnemyManagerScript;

    class RoomLockScript : public TemplateSC {
    public:

        int enemyCountToKill = 3;

        utility::GUID  doorPrefabA;
        utility::GUID  doorPrefabB;

        utility::GUID   doorSpawnPointA;
        utility::GUID   doorSpawnPointB;

        std::vector<utility::GUID> pointLightList;

        std::vector<utility::GUID> roomEnemyGUIDs;
        std::unordered_set<ecs::EntityID> enemiesInRoom;

        utility::GUID doorCloseSfxGUID;
        utility::GUID doorOpenSfxGUID;

        bool requestShake = false;
        float requestShakeIntensity = 0.f;
        float requestShakeDuration = 0.f;

        int remainingEnemies = 0;
        bool roomLocked = false;
        bool playerInside = false;

        bool isUnlocking = false;
        float doorRaiseSpeed = 4.f;
        float doorRaiseDuration = 2.f;
        float doorRaiseTimer = 0.f;
    
        // Track living enemy
        std::unordered_set<ecs::EntityID> lightInRoom;
        //std::vector<ecs::EntityID> lightInRoom;
        // Track door 
        std::vector<ecs::EntityID> spawnedDoors;

        void Start()    override;
        void Update()   override;
        void LockRoom();
        void UnlockRoom();

        REFLECTABLE(RoomLockScript, enemyCountToKill,doorPrefabA, doorPrefabB, doorSpawnPointA, doorSpawnPointB, pointLightList, roomEnemyGUIDs , doorCloseSfxGUID, doorOpenSfxGUID)
    };

    // --- IMPLEMENTATION SECTION ---
    #include "EnemyManagerScript.h"

    inline void RoomLockScript::Start() {
        // Turn off room lights initially
        for (const auto& lightGUID : pointLightList) {
            ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
            if (lightID != 0) {
                ecsPtr->SetActive(lightID, false);
            }
        }

        roomLocked = false;
        playerInside = false;
        remainingEnemies = 0;
        enemiesInRoom.clear();
        spawnedDoors.clear();

        // Register room-owned enemies from editor-assigned GUID list
        for (const auto& enemyGUID : roomEnemyGUIDs) {
            ecs::EntityID enemyID = ecsPtr->GetEntityIDFromGUID(enemyGUID);
            if (enemyID != 0) {
                enemiesInRoom.insert(enemyID);
                ecsPtr->SetActive(enemyID, false); // hide until player enters
            }
        }

        remainingEnemies = static_cast<int>(enemiesInRoom.size());

        std::cout << "[RoomLock] Start() - Registered enemies: "
            << remainingEnemies << "\n";

        // Doorway trigger: only detect player entering
        physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {
            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp) return;

            if (nameComp->entityTag == "Player") {
                if (!roomLocked) {
                    playerInside = true;
                    LockRoom();

                    std::cout << "[RoomLock] Player entered. Room locked. Enemies to kill: "
                        << enemiesInRoom.size() << "\n";

                    for (const auto& lightGUID : pointLightList) {
                        ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
                        if (lightID != 0) {
                            ecsPtr->SetActive(lightID, true);
                        }
                    }
                }
            }
            });

        physicsPtr->GetEventCallback()->OnTriggerExit(entity, [this](const physics::Collision& col) {
            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp) return;

            if (nameComp->entityTag == "Player") {
                playerInside = false;
                std::cout << "[RoomLock] Player left trigger zone.\n";
                //for (const auto& lightGUID : pointLightList) {
                //    ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
                //    if (lightID != 0) {
                //        ecsPtr->SetActive(lightID, false);
                //    }
                //}
            }
            });
    }


    inline void RoomLockScript::Update() {
        if (!roomLocked && !isUnlocking) return;

        std::vector<ecs::EntityID> toRemove;

        for (ecs::EntityID enemyID : enemiesInRoom) {
            auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(enemyID);

            if (!enemyScript) {
                std::cout << "[RoomLock] WARNING: EnemyManagerScript missing for enemyID "
                    << enemyID << "\n";
                continue;
            }

            if (enemyScript->enemyHealth <= 0) {
                toRemove.push_back(enemyID);
            }
        }

        for (ecs::EntityID id : toRemove) {
            enemiesInRoom.erase(id);
            std::cout << "[RoomLock] Dead enemy removed. Remaining: "
                << enemiesInRoom.size() << "\n";
        }

        remainingEnemies = static_cast<int>(enemiesInRoom.size());

        // Enemy dead all unlock room
        if (enemiesInRoom.empty()) {
            UnlockRoom();
        }

        // Raise doors
        if (isUnlocking) {
            doorRaiseTimer += ecsPtr->m_GetDeltaTime();

            for (ecs::EntityID doorID : spawnedDoors) {
                auto* doorTf = ecsPtr->GetComponent<TransformComponent>(doorID);
                if (!doorTf) continue;
                doorTf->LocalTransformation.position.y += doorRaiseSpeed * ecsPtr->m_GetDeltaTime();
            }

            if (doorRaiseTimer >= doorRaiseDuration) {
                isUnlocking = false;
                doorRaiseTimer = 0.f;
                std::cout << "[RoomLock] Doors fully raised.\n";
            }
        }
    }

    inline void RoomLockScript::LockRoom() {
        if (roomLocked) return;
        roomLocked = true;

        if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

            for (auto& af : ac->audioFiles) {
                if (af.audioGUID == doorCloseSfxGUID && af.isSFX) {
                    af.requestPlay = true;
                    break;
                }
            }
        }

        requestShake = true;
        requestShakeIntensity = 3.0f;
        requestShakeDuration = 0.6f;

        std::string currentScene = ecsPtr->GetSceneByEntityID(entity);

        // Spawn Door A
        if (doorPrefabA != utility::GUID{}) {
            ecs::EntityID doorA = DuplicatePrefabIntoScene<R_Scene>(currentScene, doorPrefabA);
            //spawnedDoors.push_back(doorA);

            if (doorSpawnPointA != utility::GUID{}) {
                ecs::EntityID spawnA = ecsPtr->GetEntityIDFromGUID(doorSpawnPointA);
                if (auto* spawnTf = ecsPtr->GetComponent<TransformComponent>(spawnA)) {
                    if (auto* doorTf = ecsPtr->GetComponent<TransformComponent>(doorA)) {
                        doorTf->LocalTransformation.position = spawnTf->WorldTransformation.position;
                        doorTf->LocalTransformation.rotation = spawnTf->WorldTransformation.rotation;
                    }
                }
            }
            std::cout << "[RoomLock] Door A spawned.\n";
        }

        // Spawn Door B
        if (doorPrefabB != utility::GUID{}) {
            ecs::EntityID doorB = DuplicatePrefabIntoScene<R_Scene>(currentScene, doorPrefabB);
            spawnedDoors.push_back(doorB);

            if (doorSpawnPointB != utility::GUID{}) {
                ecs::EntityID spawnB = ecsPtr->GetEntityIDFromGUID(doorSpawnPointB);
                if (auto* spawnTf = ecsPtr->GetComponent<TransformComponent>(spawnB)) {
                    if (auto* doorTf = ecsPtr->GetComponent<TransformComponent>(doorB)) {
                        doorTf->LocalTransformation.position = spawnTf->WorldTransformation.position;
                        doorTf->LocalTransformation.rotation = spawnTf->WorldTransformation.rotation;
                    }
                }
            }
            std::cout << "[RoomLock] Door B spawned.\n";
        }

        for (ecs::EntityID enemyID : enemiesInRoom) {
            ecsPtr->SetActive(enemyID, true);
        }

        std::cout << "[RoomLock] Room locked! Enemies to clear: "
            << enemiesInRoom.size() << "\n";
    }

    inline void RoomLockScript::UnlockRoom() {
        if (!roomLocked) return;

        if (auto* ac = ecsPtr->GetComponent<ecs::AudioComponent>(entity)) {

            for (auto& af : ac->audioFiles) {
                if (af.audioGUID == doorOpenSfxGUID && af.isSFX) {
                    af.requestPlay = true;
                    break;
                }
            }
        }

        requestShake = true;
        requestShakeIntensity = 1.5f;
        requestShakeDuration = 0.4f;

        std::cout << "[RoomLock] All enemies cleared! Raising doors.\n";
        roomLocked = false;
        remainingEnemies = 0;
        isUnlocking = true;
    }