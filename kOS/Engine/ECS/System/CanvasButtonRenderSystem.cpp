#include "Config/pch.h"
#include "CanvasButtonRenderSystem.h"
#include "Graphics/GraphicsManager.h"
#include "Resources/ResourceManager.h"

namespace ecs {
	void CanvasButtonRenderSystem::Init() {}
	
    void CanvasButtonRenderSystem::Update() {
		unsigned int buttonID = m_graphicsManager.buttonID;
		bool isButtonPressed = m_graphicsManager.isButtonPressed;

        const auto& entities = m_entities.Data();

        for (const EntityID id : entities) {
            TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(id);
            NameComponent* name = m_ecs.GetComponent<NameComponent>(id);
            CanvasRendererComponent* canvasRenderer = m_ecs.GetComponent<CanvasRendererComponent>(id);

            if (name->hide) { continue; }

            std::optional<std::vector<EntityID>> childEntities = m_ecs.GetChild(id);
            if (!childEntities.has_value()) { continue; }

            for (EntityID childID : childEntities.value()) {
                if (m_ecs.HasComponent<ButtonComponent>(childID)) {
                    ButtonComponent* button = m_ecs.GetComponent<ButtonComponent>(childID);
                    TransformComponent* childTransform = m_ecs.GetComponent<TransformComponent>(childID);
                    
					if (!m_ecs.HasComponent<SpriteComponent>(childID)) { continue; }
                    SpriteComponent* sprite = m_ecs.GetComponent<SpriteComponent>(childID);

					bool isHovered = buttonID == childID;
					button->isHovered = isHovered;
					button->isPressed = isHovered && isButtonPressed;

                    if (button->isPressed) { sprite->color = button->pressedColor; }
                    else if (button->isHovered) { sprite->color = button->hoveredColor; }
                    else { sprite->color = button->normalColor; }
                }
            }
        }
    }
}