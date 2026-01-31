#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class AnimationTestScript : public TemplateSC {
public:
    R_AnimController* controller = nullptr;
    AnimatorComponent* animComp = nullptr;

    void Start() 
    {
        if (animComp = ecsPtr->GetComponent<ecs::AnimatorComponent>(entity))
        {
            controller = resource->GetResource<R_AnimController>(animComp->controllerGUID).get();
            if (controller)
            {
                //Initialize animation state id with entry state id
                animComp->m_currentStateID = controller->m_EnterState->id;
                if (auto* currAnimState = controller->RetrieveStateByID(animComp->m_currentStateID))
                    currAnimState->Trigger("ForcedEntry", animComp, controller);
               /* anim->m_currentState = controller->m_EnterState;
                static_cast<AnimState*>(anim->m_currentState)->SetTrigger("ForcedEntry");*/
            }
        }

        std::cout << "ENTER Anim Script" << std::endl;
    }

    void Update() 
    {
        if (controller)
        {
            /// we change the current state using this script based on the conditions presented
            if (controller->m_EnterState == nullptr) return;


            if (TransformComponent* tc = ecsPtr->GetComponent<ecs::TransformComponent>(entity))
            {
                ///if on the left, trigger condition
                AnimatorComponent* anim = ecsPtr->GetComponent<ecs::AnimatorComponent>(entity);
                //static_cast<AnimState*>(anim->m_currentState)->SetTrigger("ForcedEntry");

                if (tc->LocalTransformation.position.x > 5.f)
                {
                    //static_cast<AnimState*>(anim->m_currentState)->SetTrigger("isGay");
                    controller->RetrieveStateByID(animComp->m_currentStateID)->Trigger("PlayerDetected", animComp, controller);
                    //currAnimState->Trigger("PlayerDetected", animComp, controller);
                }
                ///if on the right, trigger other condition
            }
        }
    }

    REFLECTABLE(AnimationTestScript);
};