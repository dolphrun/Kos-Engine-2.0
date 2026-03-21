/********************************************************************/
/*!
\file      CanvasAnimatedSpriteRenderSystem.cpp
\author    Sean Tiu (2303398)
\par       Email: s.tiu@digipen.edu
\date      Oct 01, 2025
\brief     Defines the CanvasSpriteRenderSystem class, which handles
           the rendering of 2D sprites in UI canvases within the ECS
           system.

           This system:
           - Renders screen-space attached to
             entities with CanvasRendererComponents.
           - Interfaces with the rendering pipeline to draw UI sprites
             efficiently.
           - Runs every frame to update visual elements in the user
             interface.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "Config/pch.h"
#include "ECS/ECS.h"

#include "CanvasAnimatedSpriteRenderSystem.h"
#include "Resources/ResourceManager.h"
#include "ECS/ecs.h"
#include "Graphics/GraphicsManager.h"

namespace ecs {

    void CanvasAnimatedSpriteRenderSystem::Init()
    {
        // Initialize UI rendering resources if needed
    }

    void CanvasAnimatedSpriteRenderSystem::Update()
    {
          const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            NameComponent* nameComp = m_ecs.GetComponent<NameComponent>(id);
            CanvasRendererComponent* canvas = m_ecs.GetComponent<CanvasRendererComponent>(id);


            std::optional<std::vector<EntityID>> childEntities = m_ecs.GetChild(id);
            if (!childEntities.has_value()) continue;

            //IF NOT SCREEN SPACE, push though other shit
            if (!canvas->isScreenSpace) {
                for (EntityID childID : childEntities.value())
                {
                    if (m_ecs.HasComponent<AnimatedSpriteComponent>(childID) && m_ecs.HasComponent<AnimatorComponent>(childID))
                    {
                        
                        AnimatorComponent* animComp = m_ecs.GetComponent<AnimatorComponent>(childID);
                        AnimatedSpriteComponent* spriteComp = m_ecs.GetComponent<AnimatedSpriteComponent>(childID);
                        TransformComponent* childTransform = m_ecs.GetComponent<TransformComponent>(childID);
                       
                        //Animator 

                        if (animComp->m_IsPlaying)
                        {
                            int steps = m_physicsManager.FrameCount();

                            for (int i = 0; i < steps; i++)
                            {
                                float delta = m_physicsManager.FixedDeltaTime() * animComp->m_PlaybackSpeed;

                                float totalDuration = spriteComp->spriteGUID.size() / spriteComp->framesPerSecond;

                                animComp->m_CurrentTime += delta;

                                if (spriteComp->isLooping)
                                {
                                    if (totalDuration > 0.0f)
                                        animComp->m_CurrentTime = fmod(animComp->m_CurrentTime, totalDuration);
                                }
                                else
                                {
                                    if (animComp->m_CurrentTime > totalDuration)
                                        animComp->m_CurrentTime = totalDuration;
                                }

                                spriteComp->currentFrame = static_cast<int>(
                                    animComp->m_CurrentTime * spriteComp->framesPerSecond
                                    );
                            }
                        }

                        //Sprite Part
                        
                        if (!spriteComp->spriteGUID.empty() && spriteComp->spriteGUID.size() > spriteComp->currentFrame && !spriteComp->spriteGUID[spriteComp->currentFrame].Empty())
                        {
                            std::shared_ptr<R_Texture> fontResource = m_resourceManager.GetResource<R_Texture>(spriteComp->spriteGUID[spriteComp->currentFrame]);

                            // Create sprite data
                            ScreenSpriteData spriteData{
                                childTransform->WorldTransformation.position,
                                glm::vec2{ childTransform->WorldTransformation.scale.x,
                                          childTransform->WorldTransformation.scale.y},
                                -childTransform->WorldTransformation.rotation.x,
                                spriteComp->color,
                                fontResource.get(),
                                0, 0, 0, // rows, columns, frameNumber
                                childID,
                                // NEW: UV data
                                spriteComp->uvMin,
                                spriteComp->uvMax,
                                spriteComp->useCustomUV
                            };

                            m_graphicsManager.gm_PushWorldSpriteData(std::move(spriteData));
                        }
                    }

                }
                return;
            }

            for (EntityID childID : childEntities.value())
            {

                if (m_ecs.HasComponent<AnimatedSpriteComponent>(childID) && m_ecs.HasComponent<AnimatorComponent>(childID))
                {
                    AnimatorComponent* animComp = m_ecs.GetComponent<AnimatorComponent>(childID);
                    AnimatedSpriteComponent* spriteComp = m_ecs.GetComponent<AnimatedSpriteComponent>(childID);
                    TransformComponent* childTransform = m_ecs.GetComponent<TransformComponent>(childID);


                    if (animComp->m_IsPlaying)
                    {
                        int steps = m_physicsManager.FrameCount();

                        for (int i = 0; i < steps; i++)
                        {
                            float delta = m_physicsManager.FixedDeltaTime() * animComp->m_PlaybackSpeed;

                            float totalDuration = static_cast<float>(static_cast<float>(spriteComp->spriteGUID.size()) / static_cast<float>(spriteComp->framesPerSecond));

                            animComp->m_CurrentTime += delta;

                            if (spriteComp->isLooping)
                            {
                                if (totalDuration > 0.0f)
                                    animComp->m_CurrentTime = fmod(animComp->m_CurrentTime, totalDuration);
                                
                                
                            }


                            spriteComp->currentFrame = static_cast<int>(
                                animComp->m_CurrentTime * spriteComp->framesPerSecond
                                );

                            if (animComp->m_CurrentTime > totalDuration && !spriteComp->isLooping)
                                spriteComp->currentFrame = spriteComp->spriteGUID.size() - 1;

                            
                        }
                    }
                    if (!spriteComp->spriteGUID.empty() && spriteComp->spriteGUID.size() > spriteComp->currentFrame && !spriteComp->spriteGUID[spriteComp->currentFrame].Empty())
                    {
                        std::shared_ptr<R_Texture> fontResource = m_resourceManager.GetResource<R_Texture>(spriteComp->spriteGUID[spriteComp->currentFrame]);

                        // Create sprite data
                        ScreenSpriteData spriteData{
                            childTransform->WorldTransformation.position,
                            glm::vec2{ childTransform->WorldTransformation.scale.x,
                                      childTransform->WorldTransformation.scale.y},
                            -childTransform->WorldTransformation.rotation.x,
                            spriteComp->color,
                            fontResource.get(),
                            0, 0, 0, // rows, columns, frameNumber
                            childID,
                            // NEW: UV data
                            spriteComp->uvMin,
                            spriteComp->uvMax,
                            spriteComp->useCustomUV
                        };

                        m_graphicsManager.gm_PushScreenSpriteData(std::move(spriteData));
                    }
                }
            }
        }
    }

}
