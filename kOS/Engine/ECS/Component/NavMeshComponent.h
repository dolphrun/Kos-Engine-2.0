/********************************************************************/
/*!
\file      NavMeshComponent.h
\author    Mog Shi Feng (2301239)
\par       Email: s.mog@digipen.edu
\date      Jan 31, 2026
\brief     

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef NAVMESHCOMPONENT_H
#define NAVMESHCOMPONENT_H

#include "Component.h"

namespace ecs {

    class NavMeshComponent : public Component {

    public:
        REFLECTABLE(NavMeshComponent);
    };

}

#endif // NAVMESHCOMPONENT_H