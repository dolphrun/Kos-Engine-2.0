#include "Editor.h"
#include "imgui_impl_opengl3.h"
#include "Editor/EditorCamera.h"
#include "ECS/ECS.h"
#include "ECS/ecs.h"

namespace gui {
    //floats to use
    float posSnap[3] {1.f, 1.f, 1.f};
    float rotSnap = 15.f;
    float scaleSnap = 0.1f;
    const float* snapVal;
    void ImGuiHandler::DrawGizmo(float renderPosX, float renderPosY, float renderWidth, float renderHeight)
    {
        ecs::TransformComponent* transComp = m_ecs.GetComponent<ecs::TransformComponent>(m_clickedEntityId);
        if (m_clickedEntityId < 0 || !transComp) return;

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(renderPosX, renderPosY, renderWidth, renderHeight);

        static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
        static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
        static bool useSnap{ false };
        static float snap[3] = { 1.f, 1.f, 1.f };

        if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_W) && !ImGuizmo::IsUsing())
            mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_E) && !ImGuizmo::IsUsing())
            mCurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_R) && !ImGuizmo::IsUsing())
            mCurrentGizmoOperation = ImGuizmo::SCALE;
        if (ImGui::IsWindowHovered() && ImGui::IsKeyPressed(ImGuiKey_Q) && !ImGuizmo::IsUsing())
            mCurrentGizmoMode = mCurrentGizmoMode ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
        if (ImGui::IsWindowHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            useSnap = true;
        }
        else if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl)) {
            useSnap = false;
        }

        //if (ImGui::IsKeyDown(ImGuiKey_LeftShift)) {
        //    //Move based on given offset
        //    switch (mCurrentGizmoOperation) {
        //    case ImGuizmo::TRANSLATE:
        //        snapVal = posSnap;
        //        break;;
        //    case ImGuizmo::ROTATE:
        //        snapVal = &rotSnap;
        //        break;;
        //    case ImGuizmo::SCALE:
        //        snapVal = &scaleSnap;
        //        break;;
        //    }
        //}

        glm::mat4 cameraView = m_isUi ? glm::mat4{ 1.f } : EditorCamera::editorCamera.GetViewMtx();

        //Change projection based on type... how ah
        glm::mat4 projection = m_isUi ? glm::mat4{ EditorCamera::editorCamera.GetUIOrthoMtx() } : EditorCamera::editorCamera.GetPerspMtx();
        glm::mat4 transformation = transComp->transformation;
        ImGuizmo::SetOrthographic(m_isUi ? true : false);
        // Not need for now.
        glm::mat4 deltaMtx(1.0f);

        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView), glm::value_ptr(projection),
            mCurrentGizmoOperation, mCurrentGizmoMode,
            glm::value_ptr(transformation),
            glm::value_ptr(deltaMtx),
            useSnap ? &snap[0] : NULL);

        //ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(projection), glm::value_ptr(glm::mat4(1.0f)), 200.f);

        if (ImGuizmo::IsUsing()) {
            glm::vec3 newPosition, newRotation, newScale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformation), glm::value_ptr(newPosition), glm::value_ptr(newRotation), glm::value_ptr(newScale));
            //Check if shfit is being held
            TransformSystem::SetImmediateWorldPosition(m_ecs, transComp, std::move(newPosition));
            TransformSystem::SetImmediateWorldRotation(m_ecs, transComp, std::move(newRotation));
            TransformSystem::SetImmediateWorldScale(m_ecs, transComp, std::move(newScale));
        }
    }
}