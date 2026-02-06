#include "Config/pch.h"
#include "ECS/ECS.h"

#include "AnimatorSystem.h"
#include "Resources/ResourceManager.h"

namespace ecs {

    void AnimatorSystem::Init()
    {
        // Initialize animation playback resources if needed
       /* onRegister.Add([&](EntityID id) {
            AnimatorComponent* anim = m_ecs.GetComponent<AnimatorComponent>(id);
            if (!anim) return;
            anim->m_currentState = new AnimState{};
           

            });

        onDeregister.Add([&](EntityID id) {
            AnimatorComponent* anim = m_ecs.GetComponent<AnimatorComponent>(id);
            if (!anim) return;

            if (anim->m_currentState)
            delete anim->m_currentState;

            });*/

    }

    void AnimatorSystem::Update()
    {
        const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {

            AnimatorComponent* animator = m_ecs.GetComponent<AnimatorComponent>(id);
            NameComponent* nameComp = m_ecs.GetComponent<NameComponent>(id);

            // Skip entities not in this scene or hidden
            if (nameComp->hide)
                continue;

            R_AnimController* controller{nullptr};
            R_Animation* animation{nullptr};
            AnimState* currentState{ nullptr };
            controller = m_resourceManager.GetResource<R_AnimController>(animator->controllerGUID).get();

            if (controller)
            {
                if (animator->m_transitioningStateID)
                {
                    animator->m_currentStateID = animator->m_transitioningStateID;
                    animator->m_transitioningStateID = 0;
                    animator->m_CurrentTime = 0.f;
                }

                if (animator->m_currentStateID)
                {
                    currentState = controller->RetrieveStateByID(animator->m_currentStateID); 
                    animation = m_resourceManager.GetResource<R_Animation>(currentState->animationGUID).get();
                }
                   

            }
                
            if (animation && animator->m_IsPlaying && currentState)
            {
                int steps = m_physicsManager.FrameCount();
                for (int i = 0; i < steps; i++)
                {
                    float add = (animation->GetTicksPerSecond() * m_physicsManager.FixedDeltaTime() * currentState->playSpeed) * animator->m_PlaybackSpeed * m_ecs.GetTimeScale();
                    if (currentState->isLooping)
                    {
                        animator->m_CurrentTime += add;
                        animator->m_CurrentTime = fmod(animator->m_CurrentTime, animation->GetDuration());
                    }    
                    else
                    {
                        if (animation->GetDuration() > animator->m_CurrentTime)
                            animator->m_CurrentTime += add;
                        //else
                           //animator->m_CurrentTime = fmod(animator->m_CurrentTime, animation->GetDuration());

                    }
                }
            }
        }
    }

}