/********************************************************************/
/*!
\file      imgui_hierarchy_window.cpp
\author    Ng Jaz Winn, jazwinn.ng, 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 30, 2025
\brief     This file contains the implementation of the ImGui Hierarchy Window for rendering and managing game objects.
           - Displays and updates a list of game objects (entities) in the ECS framework.
           - Provides functionality for adding, deleting, and duplicating game objects.
           - Uses ImGui to create a hierarchy view, with interaction buttons for each game object.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"

#include "ECS/ECS.h"
#include "DeSerialization/json_handler.h"
#include "Resources/ResourceManager.h"
#include "Debugging/Logging.h"
#include "AssetManager/Prefab.h"
#include "Editor/EditorCamera.h"
#include "Scene/SceneManager.h"

namespace gui
{
    
    unsigned int ImGuiHandler::DrawHierachyWindow()
    {
        std::function<void(EntityID)> updateChildScene = [&](EntityID parent)
        {
            std::string parentscene = m_ecs.GetSceneByEntityID(parent);
            const auto &child = m_ecs.GetChild(parent);

            if (child.has_value())
            {
                auto &children = child.value();
                for (auto &childid : children)
                {
                    // If child scene does not belong to parent scene, swap it
                    std::string childscene = m_ecs.GetSceneByEntityID(childid);
                    if (parentscene != childscene)
                    {
                        m_sceneManager.SwapScenes(childscene, parentscene, childid);
                    }

                    // Recursive call
                    if (m_ecs.GetChild(childid).has_value())
                    {
                        updateChildScene(childid);
                    }
                }
            }
        };

        // assetmanager::AssetManager* assetmanager = assetmanager::AssetManager::m_funcGetInstance();
        //  Custom window with example widgets
        if (ImGui::Begin("Hierachy Window", nullptr, ImGuiWindowFlags_MenuBar)) {

            static std::string searchString;
            if (m_prefabSceneMode)
                searchString.clear();

            // menu bar for search

            if (ImGui::BeginMenuBar())
            {
                ImGui::Text("Search:");
                ImGui::SameLine(); // Keep the next widget on the same line
                if (ImGui::InputText("###1234Search", &searchString))
                {
                }
                ImGui::EndMenuBar(); // End menu bar
            }

            // Add Game Object Section
            if (ImGui::Button("+ Add GameObject"))
                ImGuiHandler::m_objectNameBox ? ImGuiHandler::m_objectNameBox = false : m_objectNameBox = true;

            if (m_objectNameBox)
            {
                if (ImGui::InputText("##", m_charBuffer, IM_ARRAYSIZE(m_charBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    if (!m_activeScene.empty())
                    {
                        ecs::EntityID newEntityID = m_ecs.CreateEntity(m_activeScene);
                        m_commandHistory.AddCommand<CommandHistory::AddGameObject>(newEntityID, m_activeScene);

                        // if in prefab mode, assign entity to upmost parent
                        if (m_prefabSceneMode)
                        {
                            ecs::EntityID id = m_ecs.sceneMap.find(m_activeScene)->second.prefabID;
                            m_ecs.SetParent(std::move(id), newEntityID);
                        }

                        m_lastClickedEntityId = newEntityID;

                        m_ecs.GetComponent<ecs::NameComponent>(newEntityID)->entityName = std::string(m_charBuffer);

                        m_charBuffer[0] = '\0';
                        m_objectNameBox = false;
                    }
                    else
                    {
                        LOGGING_POPUP("No Scene Loaded");
                    }
                }
            }

            int imguiId = 0;
            static ImVec2 headerPos, headerEnd;
            for (auto& [sceneName, sceneentity] : m_ecs.sceneMap)
            {
                // when prefab mode is on, skip non prefabs, and non active prefabs
                if (m_prefabSceneMode)
                {
                    if (sceneentity.isPrefab == false || sceneentity.isActive == false)
                        continue;
                    // create seperator
                    ImGui::SeparatorText("Prefab");

                    if (ImGui::Button("Back"))
                    {
                        // save "prefab"
                        m_sceneManager.SaveScene(m_activeScene);
                        std::string prefabName = m_activeScene;
                        // set current prefab back to inactive
                        m_sceneManager.SetSceneActive(m_activeScene, false);

                        m_prefabManager.UpdateAllPrefab(m_activeScene);

                        // set back scene's active state
                        for (const auto& [scene, sceneData] : m_ecs.sceneMap)
                        {
                            if (sceneData.isPrefab == false)
                            {
                                m_sceneManager.SetSceneActive(scene, m_savedSceneState.find(scene)->second);
                            }
                        }

                        // set back active scene
                        for (auto& scene : m_ecs.sceneMap)
                        {
                            if (!scene.second.isPrefab)
                            {
                                m_activeScene = scene.first;
                                break;
                            }
                        }

                        m_prefabSceneMode = false;
                        m_lastClickedEntityId = -1;

                        renderNavMeshStatus = lastRenderStatus;
                        SetNavMeshRenderMesh();
                    }
                }

                // skip if prefab is not active
                if (sceneentity.isPrefab == true && sceneentity.isActive == false)
                    continue;
                headerPos = ImGui::GetCursorPos();
                std::string headerstr = sceneName.substr(0, sceneName.find_last_of('.'));
                // collapsing header for scene
                bool opens{};
                if (sceneentity.isActive == false)
                {
                    headerstr += " (Unloaded)";
                    ImGui::CollapsingHeader(headerstr.c_str());
                }
                else
                {
                    if (sceneName == m_activeScene)
                    {
                        headerstr += " (Active)";
                    }
                    opens = ImGui::CollapsingHeader(headerstr.c_str(), ImGuiTreeNodeFlags_DefaultOpen);
                }

                // Right Click Section
                if (ImGui::BeginPopupContextItem())
                {
                    if ((sceneName != m_activeScene) && ImGui::MenuItem("Remove Scene"))
                    {
                        m_sceneManager.ClearScene(sceneName);

                        // break loop
                        ImGui::EndPopup();
                        break;
                    }

                    if ((sceneName != m_activeScene) && (sceneentity.isActive == true) && ImGui::MenuItem("Unload Scene"))
                    {
                        m_sceneManager.SetSceneActive(sceneName, false);
                        m_lastClickedEntityId = -1;

                        if (!m_prefabSceneMode)
                        {
                            // change scene if current active scene is unloaded
                            if (m_ecs.sceneMap.find(m_activeScene)->second.isActive == false)
                            {
                                // set first loaded scene as active
                                for (auto& scene : m_ecs.sceneMap)
                                {
                                    if (scene.second.isActive == true && scene.second.isPrefab == false)
                                    {
                                        m_activeScene = m_ecs.sceneMap.begin()->first;
                                    }
                                }
                            }
                        }

                        ImGui::EndPopup();
                        break;
                    }

                    if ((sceneentity.isActive == false) && ImGui::MenuItem("load Scene"))
                    {
                        m_sceneManager.SetSceneActive(sceneName, true);
                        ImGui::EndPopup();
                        break;
                    }

                    if (ImGui::MenuItem("Save Scene"))
                    {
                        onSaveAll.Invoke(sceneName);
                    }

                    if ((sceneName != m_activeScene) && ImGui::MenuItem("Set Active"))
                    {
                        m_activeScene = sceneName;

                        if (sceneentity.isActive == false)
                        {
                            sceneentity.isActive = true;
                        }
                    }

                    ImGui::EndPopup();
                }

                // Covers the Collapsing Header Space
                if (ImGui::BeginDragDropTarget())
                {
                    auto swapScene = [&](EntityID Id) {
                        const auto& scene = m_ecs.GetSceneByEntityID(Id);

                        if (!scene.empty())
                        {
                            m_sceneManager.SwapScenes(scene, sceneName, Id);
                        }

                        // if entity is a child, break from parent
                        const auto& parent = m_ecs.GetParent(Id);
                        if (parent.has_value())
                        {

                            m_commandHistory.AddCommand<CommandHistory::SetGameObjectParent>(Id, m_ecs.GetParent(Id));
                            m_ecs.RemoveParent(Id);
                        }

                        updateChildScene(Id);
                    };

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityPayload"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(EntityPayload));
                        ecs::EntityID Id = static_cast<EntityPayload*>(payload->Data)->id;
                        swapScene(Id);
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntitiesPayload"))
                    {

                        size_t payloadCount = payload->DataSize / sizeof(ecs::EntityID);
                        const ecs::EntityID* draggedIds = static_cast<const ecs::EntityID*>(payload->Data);
                        std::unordered_set<EntityID> movingEntities;
                        for (size_t i = 0; i < payloadCount; ++i)
                        {
                            ecs::EntityID childId = draggedIds[i];
                            movingEntities.insert(childId);
                        }

                        for (auto e : movingEntities) {
                            auto parent = m_ecs.GetParent(e);
                            if (parent.has_value()) {
                                if (movingEntities.find(parent.value()) != movingEntities.end()) {
                                    continue;
                                }
                            }
                            swapScene(e);
                        }
                    }

                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                    {
                        // IM_ASSERT(payload->DataSize == sizeof(std::filesystem::path));
                        std::filesystem::path filename = static_cast<const char*>(payload->Data);

                        if (filename.filename().extension().string() == ".prefab")
                        {
                            m_prefabManager.m_CreatePrefab(filename.filename().string(), sceneName);
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                if (opens)
                {
                    // 1. Indent the root entities so there is visual space for the scene line
                    ImGui::Indent();

                    // 2. Setup line drawing properties
                    ImDrawList* drawList = ImGui::GetWindowDrawList();
                    ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();
                    verticalLineStart.x -= ImGui::GetStyle().IndentSpacing * 0.5f;
                    ImVec2 verticalLineEnd = verticalLineStart;
                    bool hasRootEntities = false;

                    for (auto entity : sceneentity.sceneIDs)
                    {

                        const auto& entityMap = m_ecs.GetEntitySignatureData();
                        if (entityMap.find(m_lastClickedEntityId) == entityMap.end())
                        {
                            m_lastClickedEntityId = entity;
                            m_isUi = false;
                        }

                        // search bar if if string not empty, must match the entity name
                        ecs::NameComponent* nc = m_ecs.GetComponent<ecs::NameComponent>(entity);
                        if (!searchString.empty() && !containsSubstring(nc->entityName, searchString))
                            continue;

                        // draw parent entity node
                        // draw entity with no parents hahaha
                        if (!m_ecs.GetParent(entity).has_value())
                        {
                            // 3. Draw horizontal ticks pointing to root entities
                            ImVec2 childPos = ImGui::GetCursorScreenPos();
                            float halfLineHeight = ImGui::GetTextLineHeight() * 0.5f;

                            drawList->AddLine(
                                ImVec2(verticalLineStart.x, childPos.y + halfLineHeight),
                                ImVec2(verticalLineStart.x + ImGui::GetStyle().IndentSpacing * 0.5f, childPos.y + halfLineHeight),
                                IM_COL32(150, 150, 150, 255)
                            );

                            verticalLineEnd.y = childPos.y + halfLineHeight;
                            hasRootEntities = true;

                            if (DrawEntityNode(entity, sceneentity.sceneIDs) == false)
                            {
                                // delete is called
                                break;
                            }
                        }
                    }

                    // 4. Draw the main vertical line connecting the Scene to its roots
                    if (hasRootEntities)
                    {
                        drawList->AddLine(verticalLineStart, verticalLineEnd, IM_COL32(150, 150, 150, 255));
                    }

                    // 5. Unindent so the rest of the UI doesn't drift to the right
                    ImGui::Unindent();

                    headerEnd = ImGui::GetCursorPos();

                    // Covers the Scene TreeNodes + Collapsing Header
                    ImGui::SetCursorPos(headerPos);
                    ImGui::PushID(imguiId++);
                    ImGui::InvisibleButton("#invbut", ImVec2{ ImGui::GetContentRegionMax().x, headerEnd.y - headerPos.y });
                    ImGui::PopID();
                    ImGui::SetCursorPos(headerEnd);
                    if (ImGui::BeginDragDropTarget())
                    {
                        auto removeParent = [&](EntityID Id) {
                            // if in prefab mode and parent does not have parent, reject
                            if (m_prefabSceneMode && m_ecs.GetParent(Id).has_value() && (!m_ecs.GetParent(m_ecs.GetParent(Id).value()).has_value()))
                            {
                            }
                            if (m_prefabSceneMode)
                            {
                                m_ecs.SetParent(m_ecs.sceneMap.find(m_activeScene)->second.prefabID, Id);
                            }
                            else
                            {
                                m_commandHistory.AddCommand<CommandHistory::SetGameObjectParent>(Id, m_ecs.GetParent(Id));
                                m_ecs.RemoveParent(Id, true);
                            }
                        };
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntityPayload"))
                        {
                            IM_ASSERT(payload->DataSize == sizeof(EntityPayload));
                            ecs::EntityID Id = static_cast<EntityPayload*>(payload->Data)->id;
                            removeParent(Id);

                        }
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntitiesPayload"))
                        {

                            size_t payloadCount = payload->DataSize / sizeof(ecs::EntityID);
                            const ecs::EntityID* draggedIds = static_cast<const ecs::EntityID*>(payload->Data);
                            std::unordered_set<EntityID> movingEntities;
                            for (size_t i = 0; i < payloadCount; ++i)
                            {
                                ecs::EntityID childId = draggedIds[i];
                                movingEntities.insert(childId);
                            }

                            for (auto e : movingEntities) {
                                auto parent = m_ecs.GetParent(e);
                                if (parent.has_value()) {
                                    if (movingEntities.find(parent.value()) != movingEntities.end()) {
                                        continue;
                                    }
                                }
                                removeParent(e);
                            }
                        }

                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                        {
                            // IM_ASSERT(payload->DataSize == sizeof(std::filesystem::path));
                            IM_ASSERT(payload->DataSize == sizeof(AssetPayload));
                            const AssetPayload* data = static_cast<const AssetPayload*>(payload->Data);

                            std::filesystem::path filePath = data->path;

                            if (!m_prefabSceneMode && filePath.filename().extension().string() == ".json")
                            {
                                m_sceneManager.LoadScene(filePath);
                            }

                            if (!m_prefabSceneMode && filePath.filename().extension().string() == ".prefab")
                            {
                                m_prefabManager.m_CreatePrefab(filePath.filename().string(), sceneName);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                }
            }
        }

        ImGui::End();
        return m_lastClickedEntityId;
    }

    bool ImGuiHandler::DrawEntityNode(ecs::EntityID id, const std::vector<EntityID>& entities)
    {

        std::function<void(EntityID)> updateChildScene = [&](EntityID parent)
        {
            std::string parentscene = m_ecs.GetSceneByEntityID(parent);
            const auto &child = m_ecs.GetChild(parent);

            if (child.has_value())
            {
                auto &children = child.value();
                for (auto &childid : children)
                {
                    // If child scene does not belong to parent scene, swap it
                    std::string childscene = m_ecs.GetSceneByEntityID(childid);
                    if (parentscene != childscene)
                    {
                        m_sceneManager.SwapScenes(childscene, parentscene, childid);
                    }

                    // Recursive call
                    if (m_ecs.GetChild(childid).has_value())
                    {
                        updateChildScene(childid);
                    }
                }
            }
        };

        ecs::TransformComponent *transCom = m_ecs.GetComponent<ecs::TransformComponent>(id);
        if (transCom == NULL)
            return false;

        bool isSelected = m_selectedEntities.find(id) != m_selectedEntities.end();
        ImGuiTreeNodeFlags flag = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;

        if (transCom->m_childID.size() <= 0)
        {
            flag |= ImGuiTreeNodeFlags_Leaf;
        }

        ecs::NameComponent *nc = m_ecs.GetComponent<ecs::NameComponent>(id);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        if (ImGui::GetContentRegionAvail().x > 0) {
            ImGui::InvisibleButton(std::string{ "##invireorderbutton" + std::to_string(id) }.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 1.f });
        }
        
        ImGui::PopStyleVar();

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityPayload"))
            {
                IM_ASSERT(payload->DataSize == sizeof(EntityPayload));
                ecs::EntityID SwapId = static_cast<EntityPayload *>(payload->Data)->id;

                const std::string swapScene = m_ecs.GetSceneByEntityID(SwapId);
                const std::string idScene = m_ecs.GetSceneByEntityID(id);

                if (SwapId != id && swapScene == idScene)
                {

                    auto &_scene = m_ecs.sceneMap.find(swapScene)->second;
                    const auto &eraseit = std::find(_scene.sceneIDs.begin(), _scene.sceneIDs.end(), SwapId);

                    if (eraseit != _scene.sceneIDs.end())
                    {
                        _scene.sceneIDs.erase(eraseit);
                    }

                    const auto &IDit = std::find(_scene.sceneIDs.begin(), _scene.sceneIDs.end(), id);

                    if (IDit != _scene.sceneIDs.end())
                    {
                        _scene.sceneIDs.insert(IDit, SwapId);
                    }
                }
            }
            ImGui::EndDragDropTarget();
        }

        // create color if prefab
        if (nc->hide && nc->isPrefab) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.1f, 0.1f, 1.0f));
        }
        else if (nc->isPrefab) {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.2f, 0.1f, 1.0f));
        }
        else if (nc->hide){
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
        }

        bool open = ImGui::TreeNodeEx(std::to_string(id).c_str(), flag, nc->entityName.c_str());
        if (ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            ImGuiIO& io = ImGui::GetIO();

            bool rightClickOnSelected = ImGui::IsItemClicked(ImGuiMouseButton_Right) && isSelected;

            if (!rightClickOnSelected)
            {
                if (io.KeyCtrl)
                {
                    // CTRL-Click: Toggle selection
                    if (isSelected)
                        m_selectedEntities.erase(id);
                    else
                        m_selectedEntities.insert(id);
                }
                else if (io.KeyShift)
                {
                    // SHIFT-Click: Range Selection
                    int firstIndex = -1;
                    int lastIndex = -1;
                    for (int n{}; n < entities.size(); ++n) {
                        auto e = entities[n];

                        if ((firstIndex == -1) && (m_lastClickedEntityId == e || id == e)) {
                            firstIndex = n;
                            continue;
                        }
                        
                        if ((firstIndex != -1) && (m_lastClickedEntityId == e || id == e)) {
                            lastIndex = n;
                            break;

                        }

                    }

                    if (firstIndex != -1 && lastIndex != -1) {

                        for (int n{firstIndex}; n <= lastIndex; ++n) {
                        
                            m_selectedEntities.insert(entities[n]);
                        }

                    }

                }
                else
                {
                    // Normal Click: Clear everything else and select this
                    m_selectedEntities.clear();
                    m_selectedEntities.insert(id);
                }
            }

            m_lastClickedEntityId = id; // Update the focus target
            m_isUi = false;
        }
        if (nc->isPrefab || nc->hide)
            ImGui::PopStyleColor();

        if (ImGui::GetIO().KeysDown[ImGuiKey::ImGuiKey_F] && m_lastClickedEntityId == id)
        {
            // EditorCamera::editorCamera.position = transCom->LocalTransformation.position;
            EditorCamera::editorCamera.target = transCom->WorldTransformation.position;
            EditorCamera::editorCamera.r = glm::length(EditorCamera::editorCamera.position - EditorCamera::editorCamera.target);
            EditorCamera::editorCamera.alpha = glm::asin((EditorCamera::editorCamera.position.y - EditorCamera::editorCamera.target.y) / EditorCamera::editorCamera.r);
            EditorCamera::editorCamera.betta = std::atan2(EditorCamera::editorCamera.position.x - EditorCamera::editorCamera.target.x, EditorCamera::editorCamera.position.z - EditorCamera::editorCamera.target.z);
            EditorCamera::editorCamera.SwitchMode(true);

            // Recompute position from spherical coordinates
            EditorCamera::editorCamera.position.x = EditorCamera::editorCamera.target.x + EditorCamera::editorCamera.r * glm::cos(EditorCamera::editorCamera.alpha) * glm::sin(EditorCamera::editorCamera.betta);
            EditorCamera::editorCamera.position.y = EditorCamera::editorCamera.target.y + EditorCamera::editorCamera.r * glm::sin(EditorCamera::editorCamera.alpha);
            EditorCamera::editorCamera.position.z = EditorCamera::editorCamera.target.z + EditorCamera::editorCamera.r * glm::cos(EditorCamera::editorCamera.alpha) * glm::cos(EditorCamera::editorCamera.betta);
        }

        // draw context window
        if (ImGui::BeginPopupContextItem())
        {
            // disable if the upmost prefab
            if (m_prefabSceneMode && (id == m_ecs.sceneMap.find(m_activeScene)->second.prefabID))
            {
            }
            else
            {
                if (ImGui::MenuItem("Delete Entity"))
                {
                    if (m_selectedEntities.find(id) == m_selectedEntities.end()) {
                        m_selectedEntities.insert(id);
                    }
                    onDelete.Invoke();
                    ImGui::EndPopup();
                    if (open)
                        ImGui::TreePop();
                    return false;
                }
            }

            if (ImGui::MenuItem("Duplicate Entity"))
            {
                if (m_selectedEntities.find(id) == m_selectedEntities.end()) {
                    m_selectedEntities.insert(id);
                }
                onDuplicate.Invoke();
                ImGui::EndPopup();
                if (open)
                    ImGui::TreePop();
                return false;
            }

            if (!m_prefabSceneMode && ImGui::MenuItem("Create Prefab"))
            {
                if (!m_prefabSceneMode)
                {
                    m_prefabManager.m_SaveEntitytoPrefab(id);
                }
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginDragDropTarget())
        {
            auto moveEntity = [&](EntityID childId) {

                if (childId == id) return;
                // dont allow prefabs to be dragged inside prefab
                const auto& childnc = m_ecs.GetComponent<ecs::NameComponent>(childId);
                const auto& parent = m_ecs.GetComponent<ecs::NameComponent>(id);

                if (!m_prefabSceneMode && childnc->isPrefab && (childnc->prefabName == parent->prefabName))
                {
                    LOGGING_WARN("Unable to drag prefabs of same type into each other, pls go to prefab editor");
                }
                else
                {
                    m_commandHistory.AddCommand<CommandHistory::SetGameObjectParent>(childId, m_ecs.GetParent(childId));
                    m_ecs.SetParent(id, childId, true);
                    LOGGING_INFO("Set Parent: %d, Child: %d", id, childId);
                    // update child's scene
                    updateChildScene(id);
                }


            };

            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityPayload"))
            {
                IM_ASSERT(payload->DataSize == sizeof(EntityPayload));
                ecs::EntityID childId = static_cast<EntityPayload *>(payload->Data)->id;

                moveEntity(childId);

                // return
                ImGui::EndDragDropTarget();
                if (open)
                    ImGui::TreePop();
                return false;
            }
            else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EntitiesPayload"))
            {

                size_t payloadCount = payload->DataSize / sizeof(ecs::EntityID);
                const ecs::EntityID* draggedIds = static_cast<const ecs::EntityID*>(payload->Data);
                std::unordered_set<EntityID> movingEntities;
                for (size_t i = 0; i < payloadCount; ++i)
                {
                    ecs::EntityID childId = draggedIds[i];
                    movingEntities.insert(childId);
                }

                for (auto e : movingEntities) {
                    auto parent = m_ecs.GetParent(e);
                    if (parent.has_value()) {
                        if (movingEntities.find(parent.value()) != movingEntities.end()) {
                            continue;
                        }
                    }
                    moveEntity(e);
                }


                // return
                ImGui::EndDragDropTarget();
                if (open)
                    ImGui::TreePop();
                return false;
            }

            ImGui::EndDragDropTarget();
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            if (m_selectedEntities.size() == 1) {
                auto* nc = m_ecs.GetComponent<ecs::NameComponent>(id);
                if (nc)
                {
                    EntityPayload payload{ id, nc->entityGUID };

                    ImGui::SetDragDropPayload("EntityPayload", &payload, sizeof(EntityPayload));
                    ImGui::Text("%s", nc->entityName.c_str());
                    if (!nc->entityGUID.Empty())
                        ImGui::Text("%s", nc->entityGUID.GetToString().c_str());
                }
                
            }
            else if (m_selectedEntities.size() > 1) {
                std::vector<ecs::EntityID> payloadData;
                payloadData.reserve(m_selectedEntities.size());
                for (auto selectedId : m_selectedEntities)
                {
                    payloadData.push_back(selectedId);
                }

                ImGui::SetDragDropPayload("EntitiesPayload",
                    payloadData.data(),
                    payloadData.size() * sizeof(ecs::EntityID));

                ImGui::Text("Moving %zu Entities", payloadData.size());
            }

            ImGui::EndDragDropSource();
        }

        if (open)
        {
            // 1. Get the draw list and calculate the start of our vertical line
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 verticalLineStart = ImGui::GetCursorScreenPos();

            // Offset X to sit right in the middle of the indentation area
            verticalLineStart.x -= ImGui::GetStyle().IndentSpacing * 0.5f;

            // Track how far down the vertical line needs to go
            ImVec2 verticalLineEnd = verticalLineStart;

            // recursion
            if (transCom->m_childID.size() > 0)
            {
                for (auto& ids : transCom->m_childID)
                {
                    // 2. Calculate horizontal tick position for each child before it draws
                    ImVec2 childPos = ImGui::GetCursorScreenPos();
                    float halfLineHeight = ImGui::GetTextLineHeight() * 0.5f;

                    // Draw horizontal line pointing right to the child node
                    drawList->AddLine(
                        ImVec2(verticalLineStart.x, childPos.y + halfLineHeight),
                        ImVec2(verticalLineStart.x + ImGui::GetStyle().IndentSpacing * 0.5f, childPos.y + halfLineHeight),
                        IM_COL32(150, 150, 150, 255)
                    );

                    // Update the bottom-most point for the vertical line
                    verticalLineEnd.y = childPos.y + halfLineHeight;

                    // Draw the actual child
                    if (!DrawEntityNode(ids, entities))
                    {
                        ImGui::TreePop();
                        return false;
                    }
                }

                // 3. Draw the main vertical line connecting all horizontal ticks
                drawList->AddLine(verticalLineStart, verticalLineEnd, IM_COL32(150, 150, 150, 255));
            }
            ImGui::TreePop();
        }
        return true;
    }
}
