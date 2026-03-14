/******************************************************************/
/*!
\file      EditorCamera.cpp
\author    Sean Tiu Gabe Ng
\par       s.tiu@digipen.edu gabe.ng@digipen.edu
\date      8th November, 2024
\brief     This file contains the implementation of the EditorCamera
           class, which provides camera control for the editor view
           in a graphical application.

           The EditorCamera is responsible for setting up and managing
           the orthographic view, including the camera's zoom, drag
           sensitivity, and aspect ratio. It provides methods to:
           - Calculate the camera transformation matrix.
           - Calculate the view matrix based on the camera position.
           - Set up an orthographic projection matrix.

           These calculations enable smooth zooming and dragging within
           the editor, ensuring the camera view can be adjusted
           interactively by the user.

           Key features of this file:
           - Configurable camera zoom and drag sensitivity.
           - Camera matrix, view matrix, and orthographic projection.
           - Static members to control aspect ratio and window size.

Copyright (C) 2024 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/



#include "../Editor/EditorCamera.h"
#include <iostream>

EditorCamera EditorCamera::editorCamera;
bool EditorCamera::m_editorMode{true};
void EditorCamera::SwitchMode(bool orbMode) {
    orbitMode = orbMode;
    //std::cout << "SWICH MODE CALLED\n" << orbitMode << orbMode;

    if (!orbMode) {
        direction = glm::normalize(target - position);
        rotation.y = glm::degrees(atan2(direction.z, direction.x)) ;
        rotation.x = glm::degrees(asin(direction.y));
        up = glm::normalize(rotation * glm::vec3(0.f, 1.f, 0.f));
        return;;
    }
    r = 5.0f;
    targetDist = minmaxTargetDist.x;
    alpha = 0.0f;
    betta = 0.0f;
}
void EditorCamera::onCursor(double xoffset, double yoffset) {
   // std::cout << "Editor type shit sir\n";
    static const float PI05 = glm::pi<float>() / 2.0f; // Half of pi for clamping
    // Sensitivity
  /*  static const float sensitivityX = 0.15f;
    static const float sensitivityY = 0.15f;*/
    //std::cout << "ORBIT MODE IS " << orbitMode << '\n';
    if (orbitMode) {
        // Increment stored angles
        alpha += static_cast<float>(yoffset) * orbitSens; // invert if needed
        betta -= static_cast<float>(xoffset) * orbitSens;

        // Clamp vertical angle
        alpha = glm::clamp(alpha, -PI05 + 0.01f, PI05 - 0.01f);

        // Recompute position from spherical coordinates
        position.x = target.x + r * glm::cos(alpha) * glm::sin(betta);
        position.y = target.y + r * glm::sin(alpha);
        position.z = target.z + r * glm::cos(alpha) * glm::cos(betta);
        return;;
    }
    
    //Do FPS rotation
    // Yaw (rotation around Y-axis)
    rotation.y += static_cast<float>(xoffset) * sens;

    // Pitch (rotation around X-axis) - Invert to match mouse movement
    rotation.x -= static_cast<float>(yoffset) * sens;
    rotation.x = glm::clamp(rotation.x, -89.f, 89.f);

    up = glm::normalize(rotation * glm::vec3(0.f, 1.f, 0.f));
}

void EditorCamera::onScroll(double xoffset, double yoffset) {
    if (targetDist <= minmaxTargetDist.x&&yoffset>0.f)return;;
    if (orbitMode) {
        r += yoffset > 0.0f ? -1.0f : 1.0f; // Zoom in or out
        if (r < 1.0f) r = 1.0f; // Clamp minimum distance

        // Update position based on new distance
        //std::cout << xoffset << ' ';
        //std::cout << yoffset << ' ';
        //std::cout << position.x << ' ';
        //std::cout << position.y << ' ';
        //std::cout << position.z << '\n';
        position.x = target.x + r * glm::cos(alpha) * glm::sin(betta);
        position.y = target.y + r * glm::sin(alpha);
        position.z = target.z + r * glm::cos(alpha) * glm::cos(betta);
        //std::cout << position.x << ' ';
        //std::cout << position.y << ' ';
        //std::cout << position.z << '\n';
        return;;
    }
    // Calculate forward direction from rotation angles
    direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    direction.y = sin(glm::radians(rotation.x));
    direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
    direction = glm::normalize(direction);

    // Move along the forward direction
    float scrollAmount = (yoffset > 0.0f ? 1.0f : -1.0f);
    targetDist += -scrollAmount;
   // std::cout << targetDist << '\n';
    position += direction * scrollAmount;
}
glm::mat4 EditorCamera::CalculateViewMtx() {
    glm::vec3 front, right, up;
    if (orbitMode) {
        front = glm::normalize(target - position);
        viewMtx = glm::lookAt(position, target, glm::vec3{ 0.0f, 1.0f, 0.0f });
    }
    else {

        direction.x = cos(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        direction.y = sin(glm::radians(rotation.x));
        direction.z = sin(glm::radians(rotation.y)) * cos(glm::radians(rotation.x));
        direction = glm::normalize(direction);
        front = direction;
        viewMtx = glm::lookAt(position, position + direction, glm::vec3{ 0.0f, 1.0f, 0.0f });
    }
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, front));
    ComputeFustrum(front, right, up);
    return viewMtx;
}

static float Deg(float r) { return glm::degrees(r); }

void EditorCamera::SnapToAxis(AxisView view, float distanceOverride) {
    float dist = (distanceOverride > 0.0f) ? distanceOverride : glm::length(position - target);
    if (dist < 0.001f) dist = 5.0f;
    glm::vec3 forward(0.0f);
    switch (view) {
    case AxisView::PosX:
        forward = glm::vec3(1, 0, 0);
        break;
    case AxisView::NegX:
        forward = glm::vec3(-1, 0, 0);
        break;
    case AxisView::PosY:
        forward = glm::vec3(0, 1, 0);
        break;
    case AxisView::NegY:
        forward = glm::vec3(0, -1, 0);
        break;
    case AxisView::PosZ:
        forward = glm::vec3(0, 0, 1);
        break;
    case AxisView::NegZ:
        forward = glm::vec3(0, 0, -1);
        break;
    }
    if (orbitMode) SwitchMode(false);
    position = target - forward * dist;
    rotation.y = Deg(atan2(forward.z, forward.x));
    rotation.x = Deg(asin(forward.y));
    rotation.x = glm::clamp(rotation.x, -89.f, 89.f);
    if (view == AxisView::PosY) up = glm::vec3(0, 0, -1);
    else if (view == AxisView::NegY) up = glm::vec3(0, 0, 1);
    else up = glm::vec3(0, 1, 0);
    direction = glm::normalize(forward);
}
void EditorCamera::SetTargetFront() {
    rotation.y = glm::degrees(atan2(direction.z, direction.x));
    rotation.x = glm::degrees(asin(direction.y));
    up = glm::normalize(rotation * glm::vec3(0.f, 1.f, 0.f));
    target = position + direction * targetDist;
}
//float EditorCamera::m_aspectRatio{};
//int EditorCamera::m_windowWidth{};
//int EditorCamera::m_windowHeight{};
//
//glm::vec2 EditorCamera::m_editorWindowPosition{};
//glm::vec2 EditorCamera::m_editorWindowDimensions{};
//glm::vec2 EditorCamera::m_gamePreviewWindowPosition{};
//glm::vec2 EditorCamera::m_gamePreviewWindowDimensions{};
//bool EditorCamera::m_editorMode = true;
//EditorCamera::Camera EditorCamera::m_editorCamera{ 1.f, 1.f ,glm::vec2(0.f,0.f), glm::vec2(5.f,5.f), 0 }; // Zoom Set To 5.f on Init
//glm::mat3x3 EditorCamera::m_editorCameraMatrix{};
//glm::mat3x3 EditorCamera::m_editorViewMatrix{};
//glm::mat3x3 EditorCamera::m_editorOrthoMatrix{};
//
//float EditorCamera::m_editorCameraDragSensitivity{ 0.01f };
//float EditorCamera::m_editorCameraZoomSensitivity{ 0.10f };
//
//void EditorCamera::calculateLevelEditorCamera()
//{
//	m_editorCameraMatrix[0][0] = m_editorCamera.zoom.x;
//	m_editorCameraMatrix[1][1] = m_editorCamera.zoom.y;
//	m_editorCameraMatrix[2][0] = m_editorCamera.m_coordinates.x;
//	m_editorCameraMatrix[2][1] = m_editorCamera.m_coordinates.y;
//	m_editorCameraMatrix[2][2] = 1;
//}
//
//void EditorCamera::calculateLevelEditorView()
//{
//    //mat3x3::Mat3Inverse(m_editorCameraMatrix, m_editorViewMatrix);
//}
//
//void EditorCamera::calculateLevelEditorOrtho()
//{
//    //if (graphicpipe::GraphicsCamera::m_aspectRatio > 0)
//    //{
//    //    float left = -1.f * (1.f / graphicpipe::GraphicsCamera::m_aspectRatio);
//    //    float right = 1.f * (1.f / graphicpipe::GraphicsCamera::m_aspectRatio);
//    //    float bottom = -1.f;
//    //    float top = 1.f;
//    //    m_editorOrthoMatrix[0][0] = (2.0f / (right - left));
//    //    m_editorOrthoMatrix[1][1] = 2.0f / (top - bottom);
//    //    m_editorOrthoMatrix[2][0] = -(right + left) / (right - left);
//    //    m_editorOrthoMatrix[2][1] = -(top + bottom) / (top - bottom);
//    //    m_editorOrthoMatrix[2][2] = 1;
//    //}
//    
//}
//
//glm::vec2 EditorCamera::calculateWorldCoordinatesFromMouse(int mouseX, int mouseY)
//{
//    //float screencoordX = mouseX - m_editorWindowPosition.x;
//    //float screencoordY = mouseY - m_editorWindowPosition.y;
//
//    //float cordX = (screencoordX - m_editorWindowDimensions.x / 2.f) / (m_editorWindowDimensions.x / 2.f);
//    //float cordY = (std::abs(screencoordY) - m_editorWindowDimensions.y / 2.f) / (m_editorWindowDimensions.y / 2.f);
//
//    //glm::vec3 translate = { cordX , -cordY, 0.f };
//    //translate.x *= EditorCamera::m_editorCameraMatrix[0][0];
//    //translate.y *= EditorCamera::m_editorCameraMatrix[1][1];
//    //if (graphicpipe::GraphicsCamera::m_aspectRatio != 0)
//    //{
//    //    translate.x *= 1.f / graphicpipe::GraphicsCamera::m_aspectRatio;
//    //}
//    //translate.x += EditorCamera::m_editorCameraMatrix[2][0];
//    //translate.y += EditorCamera::m_editorCameraMatrix[2][1];
//
//    //return glm::vec2{ translate.x, translate.y };
//
//    return glm::vec2{};
//}
