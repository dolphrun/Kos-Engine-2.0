#pragma once
#include "ECS/Component/Component.h"
#include "Config/pch.h"
#include "ScriptAdapter/TemplateSC.h"


class AnimationTestScript : public TemplateSC {
public:
    R_AnimController* controller = nullptr;

    void Start() 
    {
        if (auto* anim = ecsPtr->GetComponent<ecs::AnimatorComponent>(entity))
        {
            controller = resource->GetResource<R_AnimController>(anim->controllerGUID).get();
            if (controller)
            {
                anim->m_currentState = controller->m_EnterState;
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
                if (tc->LocalTransformation.position.x > 5.f)
                {
                    //static_cast<AnimState*>(anim->m_currentState)->SetTrigger("isGay");
                }
                ///if on the right, trigger other condition
            }
        }
    }

    REFLECTABLE(AnimationTestScript);
};