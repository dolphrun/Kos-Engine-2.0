#include "Editor.h"
#include "Editor/EditorCamera.h"


namespace gui {

    void ImGuiHandler::DrawGizmo(float renderPosX, float renderPosY, float renderWidth, float renderHeight)
    {
        ecs::TransformComponent* transComp = m_ecs.GetComponent<ecs::TransformComponent>(m_lastClickedEntityId);
        if (m_lastClickedEntityId < 0 || !transComp) return;

        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(renderPosX, renderPosY, renderWidth, renderHeight);

        static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
        static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
        static bool useSnap{ false };
        static glm::vec3 snap = { 1.f, 1.f, 1.f };

        if (ImGui::IsWindowHovered() && !ImGuizmo::IsUsing()) {
            if(ImGui::IsKeyPressed(ImGuiKey_W)){
                mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
                snap = glm::vec3(1.f);
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_E)) {
                mCurrentGizmoOperation = ImGuizmo::ROTATE;
                snap = glm::vec3(15.f);
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_R)) {
                mCurrentGizmoOperation = ImGuizmo::SCALE;
                snap = glm::vec3(0.1f);
            }
            else if (ImGui::IsKeyPressed(ImGuiKey_Q)) {
                mCurrentGizmoMode = mCurrentGizmoMode ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
            }
        }
        if (ImGui::IsWindowHovered() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
            useSnap = true;
        }
        else if (ImGui::IsKeyReleased(ImGuiKey_LeftCtrl)) {
            useSnap = false;
        }

        glm::mat4 cameraView = m_isUi ? glm::mat4{ 1.f } : EditorCamera::editorCamera.GetViewMtx();

        //Change projection based on type
        glm::mat4 projection = m_isUi ? glm::mat4{ EditorCamera::editorCamera.GetUIOrthoMtx() } : EditorCamera::editorCamera.GetPerspMtx();
        glm::mat4 transformation = transComp->transformation;
        ImGuizmo::SetOrthographic(m_isUi ? true : false);
        glm::mat4 deltaMtx(1.0f);

        // --- 1. CAPTURE ORIGINAL STATE ---
        // We decompose the matrix before manipulation to get the exact starting world transform
        glm::vec3 origPos, origRot, origScale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformation), glm::value_ptr(origPos), glm::value_ptr(origRot), glm::value_ptr(origScale));

        ImGuizmo::Manipulate(
            glm::value_ptr(cameraView), glm::value_ptr(projection),
            mCurrentGizmoOperation, mCurrentGizmoMode,
            glm::value_ptr(transformation),
            glm::value_ptr(deltaMtx),
            useSnap ? &snap[0] : NULL);

        if (ImGuizmo::IsUsing()) {
            // --- 2. GET NEW STATE FOR PRIMARY ENTITY ---
            glm::vec3 newPosition, newRotation, newScale;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transformation), glm::value_ptr(newPosition), glm::value_ptr(newRotation), glm::value_ptr(newScale));

            // Apply to Primary Entity
            TransformSystem::SetImmediateWorldPosition(m_ecs, transComp, glm::vec3(newPosition)); // Copy constructor to avoid moving the original vec3s just yet
            TransformSystem::SetImmediateWorldRotation(m_ecs, transComp, glm::vec3(newRotation));
            TransformSystem::SetImmediateWorldScale(m_ecs, transComp, glm::vec3(newScale));

            // --- 3. CALCULATE THE DELTAS ---
            glm::vec3 deltaPos = newPosition - origPos;
            glm::vec3 deltaRot = newRotation - origRot;

            // Protect against divide-by-zero for scale ratios
            glm::vec3 safeScale = origScale;
            if (abs(safeScale.x) < 0.0001f) safeScale.x = 0.0001f;
            if (abs(safeScale.y) < 0.0001f) safeScale.y = 0.0001f;
            if (abs(safeScale.z) < 0.0001f) safeScale.z = 0.0001f;
            glm::vec3 scaleRatio = newScale / safeScale;

            // --- 4. APPLY TO ALL OTHER SELECTED ENTITIES ---
            for (ecs::EntityID selectedId : m_selectedEntities)
            {
                // Skip the primary entity since we just updated it
                if (selectedId == m_lastClickedEntityId) continue;

                // HIERARCHY FILTER: Skip if this entity has a parent that is ALSO currently selected
                bool hasSelectedAncestor = false;
                auto parentOpt = m_ecs.GetParent(selectedId);
                while (parentOpt.has_value())
                {
                    if (m_selectedEntities.find(parentOpt.value()) != m_selectedEntities.end())
                    {
                        hasSelectedAncestor = true;
                        break;
                    }
                    parentOpt = m_ecs.GetParent(parentOpt.value());
                }

                // If an ancestor is selected, moving the ancestor will naturally move this child. 
                if (hasSelectedAncestor) continue;

                ecs::TransformComponent* otherTrans = m_ecs.GetComponent<ecs::TransformComponent>(selectedId);
                if (otherTrans)
                {
                    // Decompose their current world matrix to get their starting point
                    glm::vec3 otherPos, otherRot, otherScale;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(otherTrans->transformation),
                        glm::value_ptr(otherPos), glm::value_ptr(otherRot), glm::value_ptr(otherScale));

                    // Calculate final values for this entity
                    glm::vec3 finalOtherPos = otherPos + deltaPos;
                    glm::vec3 finalOtherRot = otherRot + deltaRot;
                    glm::vec3 finalOtherScale = otherScale * scaleRatio;

                    // Apply the new world values
                    TransformSystem::SetImmediateWorldPosition(m_ecs, otherTrans, std::move(finalOtherPos));
                    TransformSystem::SetImmediateWorldRotation(m_ecs, otherTrans, std::move(finalOtherRot));
                    TransformSystem::SetImmediateWorldScale(m_ecs, otherTrans, std::move(finalOtherScale));
                }
            }
        }
    }
}