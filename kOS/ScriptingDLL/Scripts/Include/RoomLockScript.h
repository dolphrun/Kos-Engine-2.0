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

    int remainingEnemies = 0;
    bool roomLocked = false;
    bool playerInside = false;

    // Track living enemy
    std::unordered_set<ecs::EntityID> enemiesInRoom;
    std::unordered_set<ecs::EntityID> lightInRoom;
    //std::vector<ecs::EntityID> lightInRoom;
    // Track door 
    std::vector<ecs::EntityID> spawnedDoors;

    void Start()    override;
    void Update()   override;
    void LockRoom();
    void UnlockRoom();

    REFLECTABLE(RoomLockScript, enemyCountToKill,doorPrefabA, doorPrefabB, doorSpawnPointA, doorSpawnPointB, pointLightList)
};

// --- IMPLEMENTATION SECTION ---
#include "EnemyManagerScript.h"

inline void RoomLockScript::Start() {

    for (const auto& lightGUID : pointLightList) {
        ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
        ecsPtr->SetActive(lightID, false);
    }

    roomLocked = false;
    playerInside = false;

    physicsPtr->GetEventCallback()->OnTriggerEnter(entity, [this](const physics::Collision& col) {

        auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
        if (!nameComp) return;

        // Player in, lock room
        if (nameComp->entityTag == "Player") {
            if (!roomLocked) {
                playerInside = true;
                LockRoom();
                std::cout << "[RoomLock] Player entered. Room locked. Enemies to kill: "
                    << remainingEnemies << "\n";
                
                for (const auto& lightGUID : pointLightList) {
                    ecs::EntityID lightID = ecsPtr-> GetEntityIDFromGUID(lightGUID);
                    ecsPtr->SetActive(lightID, true);
                }
            }
        }

        if (nameComp->entityTag == "Light") {
            lightInRoom.insert(col.otherEntityID);
            std::cout << "[RoomLock] Light in room. Tracking: " << enemiesInRoom.size() << "\n";
        }
      

        // Enemy in
        if (nameComp->entityTag == "Enemy") {
            enemiesInRoom.insert(col.otherEntityID);
            std::cout << "[RoomLock] Enemy entered room. Tracking: " << enemiesInRoom.size() << "\n";
        }
        });

        physicsPtr->GetEventCallback()->OnTriggerExit(entity, [this](const physics::Collision& col) {
            auto* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
            if (!nameComp) return;

            if (nameComp->entityTag == "Player") {
                playerInside = false;
                std::cout << "[RoomLock] Player left trigger zone.\n";
            }

            if (nameComp->entityTag == "Enemy") {
                enemiesInRoom.erase(col.otherEntityID);
                std::cout << "[RoomLock] Enemy exited trigger. Remaining: "
                    << enemiesInRoom.size() << "\n";
            }
          });

}

inline void RoomLockScript::Update() {
    if (!roomLocked) return;

    std::vector<ecs::EntityID> toRemove;

    for (ecs::EntityID enemyID : enemiesInRoom) {
        auto* enemyScript = ecsPtr->GetComponent<EnemyManagerScript>(enemyID);

        if (!enemyScript || enemyScript->enemyHealth <= 0) {
            toRemove.push_back(enemyID);
        }
    }

    for (ecs::EntityID id : toRemove) {
        enemiesInRoom.erase(id);
        std::cout << "[RoomLock] Dead enemy removed. Remaining: " << enemiesInRoom.size() << "\n";
    }

    //Enemy dead all unclok room
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

    // Spawn door B
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
    std::cout << "[RoomLock] All enemies cleared! Unlocking room.\n";

    for (ecs::EntityID doorID : spawnedDoors) {
        ecsPtr->DeleteEntity(doorID);
        std::cout << "[RoomLock] Door removed.\n";
    }

    spawnedDoors.clear();
    roomLocked = false;
}

