/********************************************************************/
/*!
\file      CanvasAnimatedSpriteRenderSystem.h
\author    Sean Tiu (2303398)
\par       Email: s.tiu@digipen.edu
\date      Oct 01, 2025
\brief     Declares the CanvasSpriteRenderSystem class, which handles
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


#ifndef CANVASANIMATEDSPRITERENDERSYS_H
#define CANVASANIMATEDSPRITERENDERSYS_H

#include "System.h"
#include "ECS/ECSList.h"

namespace ecs {

    class CanvasAnimatedSpriteRenderSystem : public ISystem {

    public:
        using ISystem::ISystem;
        void Init() override;
        void Update() override;

        REFLECTABLE(CanvasAnimatedSpriteRenderSystem)
    };

}

#endif // CANVASSPRITERENDERSYS_H