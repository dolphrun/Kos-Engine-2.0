/******************************************************************/
/*!
\file      imgui_game_scene_window.cpp
\author    Sean Tiu
\par       s.tiu@digipen.edu
\date      8th November, 2024
\brief     This file contains the implementation of the imgui game
           window, which integrates ImGui functionality for rendering
           in game elements.

           This file focuses on rendering the game scene preview
           within a dedicated ImGui window ("Game Window") while
           maintaining the correct aspect ratio for the render.
           The method m_DrawGameSceneWindow():
           - Initializes and updates the GraphicsPipe for rendering.
           - Computes the appropriate aspect ratio for the rendered
             scene to fit within the ImGui window.
           - Centers the rendered scene within the window.
           - Adjusts camera matrices to support the editor camera view.

           This file enables seamless integration of in-game scenes
           within an editor window, making it possible for users to
           preview gameplay directly from the editor.

           Key features:
           - Dynamic aspect ratio handling for scene previews.
           - Integration with GraphicsPipe and EditorCamera for
             scene rendering.
           - Use of ImGui's AddImage for displaying textures.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"
#include "ECS/ECS.h"
#include "Application/Application.h"
#include "Editor/EditorCamera.h"
#include "Graphics/GraphicsManager.h"

#include <imgui_internal.h>

namespace gui
{
	void ImGuiHandler::DrawGameSceneWindow(unsigned int windowWidth, unsigned int windowHeight)
	{
        ImGui::Begin("Game Preview");

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 renderWindowSize = ImGui::GetContentRegionAvail();
        float renderWindowAspectRatio = renderWindowSize.x / renderWindowSize.y;
        float textureAspectRatio = (float)windowWidth / (float)windowHeight;

        ImVec2 imageSize;

        //Dynamic Window Resizing
        if (renderWindowAspectRatio > textureAspectRatio)
        {
            imageSize.y = renderWindowSize.y;
            imageSize.x = imageSize.y * textureAspectRatio;
        }
        else
        {
            imageSize.x = renderWindowSize.x;
            imageSize.y = imageSize.x / textureAspectRatio;
        }

        if (imageSize.x <= renderWindowSize.x)
        {
            pos.x += (renderWindowSize.x - imageSize.x) / 2;
        }

        if (imageSize.y <= renderWindowSize.y)
        {
            pos.y += (renderWindowSize.y - imageSize.y) / 2;
        }

        //pipe->m_renderFinalPassWithDebug();
        gameWindowPos = pos;
        gameWindowSize = imageSize;
        ImVec2 pMax(pos.x + imageSize.x, pos.y + imageSize.y);

		const FrameBuffer* fbAdd = &m_graphicsManager.gm_GetGameBuffer();

        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<void*>(static_cast<uintptr_t>(fbAdd->texID)),
            pos, pMax,
            ImVec2(0, 1), ImVec2(1, 0));
        
        if (ImGui::IsWindowHovered()) {
			ImVec2 mousePos = ImGui::GetIO().MousePos;
			m_graphicsManager.isButtonPressed = ImGui::IsMouseDown(ImGuiMouseButton_Left);

			float relX = (mousePos.x - pos.x) / (pMax.x - pos.x);
			float relY = (mousePos.y - pos.y) / (pMax.y - pos.y);

            if (relX >= 0.0f && relX <= 1.0f && relY >= 0.0f && relY <= 1.0f) {
                int pixelX = static_cast<int>(relX * fbAdd->width);
                int pixelY = -(static_cast<int>(relY * fbAdd->height) - fbAdd->height);
                
                GLuint fbo;
				glGenFramebuffers(1, &fbo);
				glBindFramebuffer(GL_FRAMEBUFFER, fbo);
				glFramebufferTexture2D(
                    GL_FRAMEBUFFER, 
                    GL_COLOR_ATTACHMENT0, 
                    GL_TEXTURE_2D, 
                    m_graphicsManager.gm_GetFBM()->gBuffer.gMaterial, 
                    0
                );

                float pixelVal;
                glReadPixels(
                    pixelX, 
                    pixelY, 
                    1, 
                    1, 
                    GL_ALPHA, 
                    GL_FLOAT, 
                    &pixelVal
				);

				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glDeleteFramebuffers(1, &fbo);

				--pixelVal; 
				m_graphicsManager.buttonID = (pixelVal < 0.0f) ? -1 : static_cast<int>(pixelVal);
            }
        }

        auto winLoc = ImVec2(pos.x - ImGui::GetWindowPos().x, pos.y - ImGui::GetWindowPos().y);
        ImGui::SetCursorPos(winLoc);
        if ((imageSize.x != 0 && imageSize.y != 0) && m_ecs.GetState() == GAMESTATE::RUNNING && ImGui::InvisibleButton("##GameWindowBut", imageSize, ImGuiButtonFlags_MouseButtonLeft) ) {
            m_input.HideCursor(true);
            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
        }

        if (m_input.cursorHidden) {
            ImGui::SetMouseCursor(ImGuiMouseCursor_None);
            if (ImGui::IsKeyDown(ImGuiKey_Escape)) {
                m_input.HideCursor(false);
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                ImGuiIO& io = ImGui::GetIO();
                io.ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            }
        }

        //ImGui::SetCursorPos(winLoc);
        //auto str1 = "WinPos [" + std::to_string(winLoc.x) + ',' + std::to_string(winLoc.y) + "] Pos: [" + std::to_string(pos.x) + ',' + std::to_string(pos.y) + "]";
        //ImGui::Text(str1.c_str());

        //ImGui::SetCursorPosY(winLoc.y + imageSize.y);
        //auto str = std::to_string(ImGui::GetCursorPos().x) + ',' + std::to_string(ImGui::GetCursorPos().y);
        //ImGui::Text(str.c_str());
   
        //auto str2 = std::to_string(pMax.x) + ',' + std::to_string(pMax.y);
        //auto size = ImGui::CalcTextSize(str.c_str());
        //ImGui::SetCursorPos(ImVec2(pMax.x - size.x - ImGui::GetWindowPos().x, pMax.y - ImGui::GetWindowPos().y));
        //ImGui::Text(str2.c_str());

        // Draw "Game Mouse Position"
        //ImGui::GetWindowDrawList()->AddCircleFilled(
        //    ImVec2(gameWindowPos.x + m_input.currentMousePos.x, gameWindowPos.y + (gameWindowSize.y - m_input.currentMousePos.y)),
        //    4.0f, IM_COL32(255, 0, 0, 255));

        ImGui::End();
	}
}