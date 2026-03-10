/******************************************************************/
/*!
\file      imgui_editor_window.cpp
\author    Sean Tiu
\par       s.tiu@digipen.edu
\date      8th November, 2024
\brief     This file implements the editor window interface for the
           application, using ImGui for the graphical user interface.

           The editor window provides an interactive interface for the
           user to manipulate and view objects in the application's
           scene, such as entities, cameras, and textures. It includes
           functionality for rendering the scene in a window, zooming,
           dragging, and resetting the camera view. Additionally, it
           supports drag-and-drop of assets (such as .png, .ttf, and
           .prefab files) into the editor window, automatically creating
           entities and setting their properties based on the file type.

           Key features of this file:
           - Draws the editor window and render screen.
           - Handles camera zoom, drag, and reset controls.
           - Manages drag-and-drop functionality for asset files.
           - Updates camera matrices and view transformations.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/



#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"
#include "imgui_internal.h"
#include "Application/Application.h"


#include "Editor/EditorCamera.h"
#include "Resources/ResourceManager.h"
#include "AssetManager/Prefab.h"

#include "Graphics/GraphicsManager.h"


void gui::ImGuiHandler::DrawRenderScreenWindow(unsigned int windowWidth, unsigned int windowHeight)
{
    for (const auto& line : m_physicsManager.m_debugRays) {
        m_graphicsManager.gm_PushLineDebugData(DebugLineData{ line.start, line.end, line.color });
    }
    m_physicsManager.m_debugRays.clear();
    for (const auto& sphere : m_physicsManager.m_debugSpheres) {
        glm::mat4 model = glm::translate(glm::mat4{ 1.0f }, sphere.center) * glm::scale(glm::mat4{ 1.0f }, glm::vec3{ sphere.radius });
        m_graphicsManager.gm_PushSphereDebugData(BasicDebugData{ model, sphere.color });
    }
    m_physicsManager.m_debugSpheres.clear();

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_MenuBar;
    bool open = true;

    if (ImGui::Begin("Editor Window", &open, window_flags)) {

        DrawPlayPauseBar();

        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 renderWindowSize = ImGui::GetContentRegionAvail();

        float textureAspectRatio = (float)windowWidth / (float)windowHeight;
        float renderWindowAspectRatio = renderWindowSize.x / renderWindowSize.y;

        ImGuiIO& io = ImGui::GetIO();

        ImVec2 imageSize;
        imageSize.x = windowWidth / 2.f;
        imageSize.y = windowHeight / 2.f;

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
        ImVec2 pMax(pos.x + imageSize.x, pos.y + imageSize.y);

        // std::cout << imageSize.x << ' ' << imageSize.y;
        //ImGui::GetWindowDrawList()->AddImage(
        //    (void*)(GLuint)GraphicsManager::GetInstance()->gm_GetEditorBuffer().texID,
        //    pos, pMax,
        //    ImVec2(0, 1), ImVec2(1, 0));
        const FrameBuffer* fbAdd = &m_graphicsManager.gm_GetEditorBuffer();
        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<void*>(static_cast<uintptr_t>(fbAdd->texID)),
            pos, pMax,
            ImVec2(0, 1), ImVec2(1, 0));

        //Get mouse position
        static bool lAltPressed{false};
        ImVec2 mousePos = ImGui::GetIO().MousePos;
        if (ImGui::IsKeyPressed(ImGuiKey_LeftAlt)) {
            lAltPressed = true;
        }
        else if (ImGui::IsKeyReleased(ImGuiKey_LeftAlt)) {
            lAltPressed = false;
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGuizmo::IsOver()&&!lAltPressed) {
            std::cout << "CLICKING EDITOR WINDOW\n";
            //GLenum err = glGetError();
            //if (err != GL_NO_ERROR) {
            //    //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
            //    std::cout << "before OpenGL Error: " << err << std::endl;
            //}            // Mouse click relative to image
            float relX = (mousePos.x - pos.x) / (pMax.x - pos.x);
            float relY = (mousePos.y - pos.y) / (pMax.y - pos.y);
            //Clamp
            relX = std::clamp(relX, 0.0f, 1.0f);
            relY = std::clamp(relY, 0.0f, 1.0f);

            int pixelX = static_cast<int>(relX * fbAdd->width);
            int pixelY = -(static_cast<int>(relY * fbAdd->height) - fbAdd->height);

            //Get texture data
            GLuint fbo;
            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            //err = glGetError();
            //if (err != GL_NO_ERROR) {
            //    //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
            //    std::cout << "after OpenGL Error3: " << err << std::endl;
            //}
            // Bind your texture to the FBO
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_graphicsManager.gm_GetFBM()->gBuffer.gMaterial, 0);
            // Read just one pixel
            //err = glGetError();
            //if (err != GL_NO_ERROR) {
            //    //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
            //    std::cout << "after OpenGL Error2: " << err << std::endl;
            //}
            float pixelVal;
            glReadPixels(pixelX, pixelY, 1, 1, GL_ALPHA, GL_FLOAT, &pixelVal);
            //err = glGetError();
            //if (err != GL_NO_ERROR) {
            //    //LOGGING_ERROR("First OpenGL Error: 0x%X", err);h
            //    std::cout << "after OpenGL Error 1: " << err << std::endl;
            //}
            --pixelVal;
            std::cout << "Clicked pixerl val is " << pixelVal << '\n';
            m_lastClickedEntityId = pixelVal >= 0.f ? static_cast<int>(pixelVal) : m_lastClickedEntityId;
            if (!io.KeyCtrl) {
                m_selectedEntities.clear();
            }
            if (pixelVal >= 0.f) {
                m_selectedEntities.insert(m_lastClickedEntityId);
            }
            //std::cout << "PixelVal is " << pixelVal << '\n';
            if (m_ecs.HasComponent<ecs::CanvasRendererComponent>(static_cast<EntityID>(pixelVal))
                || (m_ecs.GetParent(m_lastClickedEntityId).has_value() &&
                    m_ecs.HasComponent<ecs::CanvasRendererComponent>(m_ecs.GetParent(m_lastClickedEntityId).value()))) {
                /*std::cout << "IS UI\n";*/
                m_isUi = true;
            }
            else {
                m_isUi = false;
            }
            //Get texture
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDeleteFramebuffers(1, &fbo);

        }

        DrawGizmo(pos.x, pos.y, imageSize.x, imageSize.y);

        float scrollInput = io.MouseWheel; // Positive for zoom in, negative for zoom out

        //Zoom In/Out Camera
        if (ImGui::IsWindowHovered() && scrollInput)
        {
            EditorCamera::editorCamera.onScroll(0, scrollInput);
        }
        //For camera stuff
        static bool mouseCon = false;
        static ImVec2 lastMousePos = ImVec2(0, 0);
        static bool firstMouseInput = true;
        static bool lAltReleased = false;
        if ((ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) && ImGui::IsWindowHovered())
        {
            mouseCon = true;
            firstMouseInput = true;
            lastMousePos = ImGui::GetMousePos();
        }
        else if ((ImGui::IsMouseReleased(ImGuiMouseButton_Right) || ImGui::IsMouseReleased(ImGuiMouseButton_Middle)))
        {
            mouseCon = false;
        }
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && lAltPressed) {
            mouseCon = true;
            firstMouseInput = true;
            lastMousePos = ImGui::GetMousePos();
            EditorCamera::editorCamera.SetTargetFront();

            // Initialize spherical coordinates based on current position/target
            glm::vec3 offset = EditorCamera::editorCamera.position - EditorCamera::editorCamera.target;
            EditorCamera::editorCamera.r = glm::length(offset);
            if (EditorCamera::editorCamera.r < 0.001f) EditorCamera::editorCamera.r = 5.0f;

            glm::vec3 offsetNorm = glm::normalize(offset);
            EditorCamera::editorCamera.alpha = asin(offsetNorm.y);
            EditorCamera::editorCamera.betta = atan2(offsetNorm.x, offsetNorm.z);

            EditorCamera::editorCamera.orbitMode = true;
            lAltReleased = true;
        }
        else if(ImGui::IsMouseReleased(ImGuiMouseButton_Left)&&lAltReleased) {
            EditorCamera::editorCamera.direction = glm::normalize(EditorCamera::editorCamera.target - EditorCamera::editorCamera.position);
            EditorCamera::editorCamera.SwitchMode(false);
            mouseCon = false;
            lAltReleased = false;
        }
        //Do alt left click

        if (mouseCon)
        {
            ImVec2 currentMousePos = ImGui::GetMousePos();
            const float cameraSpeed = 0.1f; // adjust accordingly
            float sprintMultiplier = 1.f;
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
                sprintMultiplier = 2.5f;
            }
            else {
                sprintMultiplier = 1.f;
            }

            if (ImGui::IsKeyDown(ImGuiKey_W)) {
                if (EditorCamera::editorCamera.orbitMode) EditorCamera::editorCamera.SwitchMode(false);
                EditorCamera::editorCamera.position += sprintMultiplier * cameraSpeed * EditorCamera::editorCamera.direction;
                EditorCamera::editorCamera.target = sprintMultiplier * cameraSpeed * EditorCamera::editorCamera.direction;
            }
            if (ImGui::IsKeyDown(ImGuiKey_S)) {
                if (EditorCamera::editorCamera.orbitMode) EditorCamera::editorCamera.SwitchMode(false);
                EditorCamera::editorCamera.position -= sprintMultiplier * cameraSpeed * EditorCamera::editorCamera.direction;
                EditorCamera::editorCamera.target -= sprintMultiplier * cameraSpeed * EditorCamera::editorCamera.direction;

            }
            if (ImGui::IsKeyDown(ImGuiKey_A)) {
                if (EditorCamera::editorCamera.orbitMode) EditorCamera::editorCamera.SwitchMode(false);
                EditorCamera::editorCamera.position -= sprintMultiplier * cameraSpeed * glm::normalize(glm::cross(EditorCamera::editorCamera.direction, glm::vec3{ 0.0f, 1.0f, 0.0f }));
                EditorCamera::editorCamera.target -= sprintMultiplier * cameraSpeed * glm::normalize(glm::cross(EditorCamera::editorCamera.direction, glm::vec3{ 0.0f, 1.0f, 0.0f }));

            }
            if (ImGui::IsKeyDown(ImGuiKey_D)) {
                if (EditorCamera::editorCamera.orbitMode)EditorCamera::editorCamera.SwitchMode(false);
                EditorCamera::editorCamera.position += sprintMultiplier * cameraSpeed * glm::normalize(glm::cross(EditorCamera::editorCamera.direction, glm::vec3{ 0.0f, 1.0f, 0.0f }));
                EditorCamera::editorCamera.target += sprintMultiplier * cameraSpeed * glm::normalize(glm::cross(EditorCamera::editorCamera.direction, glm::vec3{ 0.0f, 1.0f, 0.0f }));
            }
            if (!firstMouseInput)
            {
                float deltaX = currentMousePos.x - lastMousePos.x;
                float deltaY = currentMousePos.y - lastMousePos.y;

                // Call your camera function with the delta
                if (ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    EditorCamera::editorCamera.onCursor(deltaX, deltaY);
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                    if (std::abs(deltaX) > 0.1f || std::abs(deltaY) > 0.1f) {
                        if (EditorCamera::editorCamera.orbitMode) EditorCamera::editorCamera.SwitchMode(false);
                        glm::vec3 forward;
                        forward.x = cos(glm::radians(EditorCamera::editorCamera.rotation.y)) * cos(glm::radians(EditorCamera::editorCamera.rotation.x));
                        forward.y = sin(glm::radians(EditorCamera::editorCamera.rotation.x));
                        forward.z = sin(glm::radians(EditorCamera::editorCamera.rotation.y)) * cos(glm::radians(EditorCamera::editorCamera.rotation.x));
                        forward = glm::normalize(forward);
                        glm::vec3 right = glm::normalize(glm::cross(glm::vec3{ 0.0f, 1.0f, 0.0f }, forward));
                        glm::vec3 up = glm::normalize(glm::cross(forward, right));
                        EditorCamera::editorCamera.position += (deltaX * right + deltaY * up) * cameraSpeed;
                    }
                }
                else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)&&lAltPressed) {
                    EditorCamera::editorCamera.onCursor(deltaX, deltaY);
                }
            }
            lastMousePos = currentMousePos;
            firstMouseInput = false;
        }


        if (io.KeysDown[ImGuiKey::ImGuiKey_F])
        {
            ecs::TransformComponent* transCom = m_ecs.GetComponent<ecs::TransformComponent>(m_lastClickedEntityId);
            if (transCom != NULL) {
                // EditorCamera::editorCamera.position = transCom->LocalTransformation.position;
                EditorCamera::editorCamera.target = transCom->WorldTransformation.position;
                EditorCamera::editorCamera.r = 5.0f;
                EditorCamera::editorCamera.targetDist = EditorCamera::editorCamera.minmaxTargetDist.x;
                EditorCamera::editorCamera.SwitchMode(true);
                EditorCamera::editorCamera.SwitchMode(false);
            }
        }
        // Uncomment this block to enable Unity-style 2d view snapping thingy it's not the best but it does it's purpose for now
        // I'm gonna finger your asshole Gabe.
        //bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
        //if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::NegX);
        //if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::PosX);
        //if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::PosY);
        //if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::NegY);
        //if (shift) {
        //    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))   EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::NegZ);
        //    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) EditorCamera::editorCamera.SnapToAxis(EditorCamera::AxisView::PosZ);
        //}
        // 

        //static unsigned int lastEntity{};

        //// Clean up behaviours when switching entities
        //if (static_cast<int>(lastEntity) != m_lastClickedEntityId) {
        //    lastEntity = m_lastClickedEntityId;
        //    m_collisionSetterMode = false;
        //}
    }

    ImGui::End();
}
