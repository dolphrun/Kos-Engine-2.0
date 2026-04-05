#pragma once
#include "ScriptAdapter/TemplateSC.h"
#include "RoomLockScript.h"

class TurnOffLight : public TemplateSC {
public:
    utility::GUID roomLock;
    std::vector<utility::GUID> listOfLights;
    void Start()    override;
    REFLECTABLE(TurnOffLight,roomLock, listOfLights)
};
inline void TurnOffLight::Start() {
    physicsPtr->GetEventCallback()->OnTriggerExit(entity, [this](const physics::Collision& col) {
        NameComponent* nameComp = ecsPtr->GetComponent<NameComponent>(col.otherEntityID);
        if (!nameComp) return;

        if (nameComp->entityTag == "Player") {
            std::cout << "[TurnOffLight] Player left trigger zone.\n";
            //Get list of point loights from roomLock
            ecs::EntityID eID = ecsPtr->GetEntityIDFromGUID(roomLock);

            if (RoomLockScript* roomLockScript = ecsPtr->GetComponent<RoomLockScript>(eID)) {
                for (const auto& lightGUID : roomLockScript->pointLightList) {
                    ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
                    if (lightID != 0) {
                        ecsPtr->SetActive(lightID, false);
                    }
                }
            }
            for (const auto& lightGUID : listOfLights) {
                ecs::EntityID lightID = ecsPtr->GetEntityIDFromGUID(lightGUID);
                if (lightID != 0) {
                    ecsPtr->SetActive(lightID, false);
                }
            }
        }
        });
}
