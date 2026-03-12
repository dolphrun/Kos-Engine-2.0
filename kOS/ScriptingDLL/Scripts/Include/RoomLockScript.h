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

    int remainingEnemies = 0;
    bool roomLocked = false;
    bool playerInside = false;

    // Track living enemy
    std::unordered_set<ecs::EntityID> lightInRoom;
    //std::vector<ecs::EntityID> lightInRoom;
    // Track door 
    std::vector<ecs::EntityID> spawnedDoors;

    void Start()    override;
    void Update()   override;
    void LockRoom();
    void UnlockRoom();

    REFLECTABLE(RoomLockScript, enemyCountToKill,doorPrefabA, doorPrefabB, doorSpawnPointA, doorSpawnPointB, pointLightList, roomEnemyGUIDs)
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
        }
        });
}


inline void RoomLockScript::Update() {
    if (!roomLocked) return;

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
}

inline void RoomLockScript::LockRoom() {
    if (roomLocked) return;
    roomLocked = true;

    std::string currentScene = ecsPtr->GetSceneByEntityID(entity);

    // Spawn Door A
    if (doorPrefabA != utility::GUID{}) {
        ecs::EntityID doorA = DuplicatePrefabIntoScene<R_Scene>(currentScene, doorPrefabA);
        spawnedDoors.push_back(doorA);

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

    std::cout << "[RoomLock] Room locked! Enemies to clear: "
        << enemiesInRoom.size() << "\n";
}

inline void RoomLockScript::UnlockRoom() {
    if (!roomLocked) return;

    std::cout << "[RoomLock] All enemies cleared! Unlocking room.\n";

    for (ecs::EntityID doorID : spawnedDoors) {
        ecsPtr->DeleteEntity(doorID);
        std::cout << "[RoomLock] Door removed.\n";
    }

    spawnedDoors.clear();
    roomLocked = false;
    remainingEnemies = 0;
}