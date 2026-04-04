/******************************************************************/
/*!
\file      imgui_global_settings_panel.cpp
\author    Sean Tiu, s.tiu , 2303398
\par       s.tiu@digipen.edu
\date      31 January 2025
\brief     This file helps with implementing global settings for the editor

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"
#include "implot.h"
#include "Debugging/Logging.h"
#include "ECS/Layers.h"




void gui::ImGuiHandler::DrawSceneSettingsWindow() 
{
    if (ImGui::Begin("Scene Settings", nullptr, ImGuiWindowFlags_HorizontalScrollbar)) {
        if (m_ecs.sceneMap.find(m_activeScene) != m_ecs.sceneMap.end()) {

            static utility::GUID lastGuid;
            SceneData& selectedScene = m_ecs.sceneMap.at(m_activeScene);
            lastGuid = selectedScene.postProcessingProfile;
            m_ecs.sceneMap.at(m_activeScene).ApplyFunction(DrawComponents{ selectedScene.Names() });
            if (lastGuid != selectedScene.postProcessingProfile)m_graphicsManager.postProcessProfile = &m_resourceManager.GetResource<R_PostProcessingProfile>(selectedScene.postProcessingProfile)->profile;
        }

    }
    



    ImGui::End();
}

    