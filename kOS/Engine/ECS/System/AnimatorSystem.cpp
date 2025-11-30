#include "Config/pch.h"
#include "ECS/ECS.h"

#include "AnimatorSystem.h"
#include "Resources/ResourceManager.h"

namespace ecs {

    void AnimatorSystem::Init()
    {
        // Initialize animation playback resources if needed
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

            R_AnimController* controller{};
            R_Animation* animation{};

            controller = m_resourceManager.GetResource<R_AnimController>(animator->controllerGUID).get();

            if (controller)
            {
                if (animator->m_currentState)
                {
                    std::vector<AnimTransition>& transitions = static_cast<AnimState*>(animator->m_currentState)->outgoingTransitions;
                    for (const AnimTransition& transition : transitions)
                    {
                        if (static_cast<AnimState*>(animator->m_currentState)->CanTransition(transition))
                        {
                            animator->m_currentState = controller->FindStateFromPin(transition.toPinId);
                            animator->m_CurrentTime = 0.f;
                            if (animator->m_currentState == nullptr) return;
                            break;
                        }
                    }
                    
                    animation = m_resourceManager.GetResource<R_Animation>(static_cast<AnimState*>(animator->m_currentState)->animationGUID).get();
                }
                   

            }
                
            if (animation && animator->m_IsPlaying)
            {
                int steps = m_physicsManager.FrameCount();
                for (int i = 0; i < steps; i++)
                {
                    animator->m_CurrentTime += (animation->GetTicksPerSecond() * m_physicsManager.FixedDeltaTime() * static_cast<AnimState*>(animator->m_currentState)->playSpeed) * animator->m_PlaybackSpeed;
                    if (static_cast<AnimState*>(animator->m_currentState)->isLooping)
                        animator->m_CurrentTime = fmod(animator->m_CurrentTime, animation->GetDuration());
                }
            }
        }
    }

}