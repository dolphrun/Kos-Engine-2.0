/******************************************************************/
/*!
\file      Imgui_performance_window.cpp
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 02, 2025
\brief     This file contains the performance window that prints out
           the time taken for all systems in each frame.



Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"
#include "Debugging/Logging.h"
#include "Debugging/Performance.h"


void gui::ImGuiHandler::DrawPerformanceWindow() {


    ImGui::Begin("Performance");

    static float FpsValues[90] = {};
    static int FpsValues_offset = 0;
    static double refresh_time = 0.0;
    static auto categoryPeformance = m_performance.GetSystemPerformance();


    if (refresh_time == 0.0)
        refresh_time = ImGui::GetTime();
    while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate
    {
        static float phase = 0.0f;
        FpsValues[FpsValues_offset] = m_performance.GetFPS();
        FpsValues_offset = (FpsValues_offset + 1) % IM_ARRAYSIZE(FpsValues);
        phase += 0.10f * FpsValues_offset;
        refresh_time += 1.0f / 60.0f;
    }

    {
        float average = 0.0f;
        for (int n = 0; n < IM_ARRAYSIZE(FpsValues); n++)
            average += FpsValues[n];
        average /= (float)IM_ARRAYSIZE(FpsValues);
        char overlay[32];
        sprintf_s(overlay, "FPS %f", average);

        float minFps = FLT_MAX;
        float maxFps = -FLT_MAX;

        for (int n = 0; n < IM_ARRAYSIZE(FpsValues); n++) {
            if (FpsValues[n] < minFps) minFps = FpsValues[n];
            if (FpsValues[n] > maxFps) maxFps = FpsValues[n];
        }

        // Ensure there is at least some margin on the Y-axis
        float yMin = minFps - 1.0f;
        float yMax = maxFps + 1.0f;
        
        //TODO change to ImPlot
        ImGui::PlotLines("##", FpsValues, IM_ARRAYSIZE(FpsValues), FpsValues_offset, overlay, yMin, yMax, ImVec2(260.f, 120.0f));

       // ImGui::PlotShaded();
    }

    static float interval = 0.0f;
    static std::unordered_map<std::string, float> systemPeformance;

    // Always get fresh data
    interval += m_performance.GetDeltaTime();
    if (interval > 1.0f) { // updates display every 1 second
        categoryPeformance = m_performance.GetSystemPerformance(); // update map to display
        interval = 0.0f;
    }

    for (const auto& [categoryName, category] : categoryPeformance) {
        if (ImGui::CollapsingHeader(categoryName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const auto& [systemName, duration] : category) {
                ImGui::Text("%s", systemName.c_str());
                ImGui::SameLine(300);
                ImGui::Text("%.4f ms", duration * 1000.0f);
            }
        }

    }


    ImGui::End();

}