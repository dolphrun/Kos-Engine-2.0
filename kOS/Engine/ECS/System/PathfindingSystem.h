#pragma once

/******************************************************************/
/*!
\file      PathfindingSystem.h
\author    Yeo See Kiat Raymond, seekiatraymond.yeo, 2301268
\par       seekiatraymond.yeo@digipen.edu
\date      October 3, 2025
\brief     This file contains the pathfinding system to draw wireframe


Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#ifndef PATHFINDINGSYSTEM_H
#define PATHFINDINGSYSTEM_H

#include "ECS/ECS.h"
#include "System.h"

namespace ecs {
	class PathfindingSystem : public ISystem {

	public:
		using ISystem::ISystem;


		void Init() override;
		void Update() override;
		REFLECTABLE(PathfindingSystem)
	};
}

#endif