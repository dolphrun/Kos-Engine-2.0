/******************************************************************/
/*!
\file      imgui_component_window.cpp
\author    Jaz Winn Ng, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 02, 2025
\brief     This file implements the ImGui component window for handling
           user interactions with ECS components in the application. It
           enables the addition, display, and modification of various
           entity components within the ECS framework.

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
#include "Resources/ResourceManager.h"
#include "AssetManager/Prefab.h"
#include "ECS/Layers.h"
#include "Editor/WindowFile.h"

//scripts
#include "Reflection/Field.h"


void gui::ImGuiHandler::DrawComponentWindow()
{
    bool windowOpen = true;
    if (ImGui::Begin("Component Window", &windowOpen)) {
        //Add Component Window

        if (m_ecs.GetEntitySignatureData().size() > 0 && m_lastClickedEntityId >= 0) {

            ecs::EntityID entityID = m_lastClickedEntityId;
            ecs::ComponentSignature EntitySignature = m_ecs.GetEntitySignature(entityID);

            static std::vector<const char*>componentNames;
            static const auto& componentsString = m_ecs.GetComponentsString();
            if (componentNames.size() != componentsString.size()) {
                componentNames.clear();
                for (const auto& names : componentsString) {
                    componentNames.push_back(names.c_str());
                }
            }

            static ImGuiTextFilter componentFilter;
            static int selectedIndex = -1;

            if (ImGui::Button("Add Component"))
            {
                ImGui::OpenPopup("AddComponentPopup");
            }

            if (ImGui::BeginPopup("AddComponentPopup"))
            {
                componentFilter.Draw("Search");

                ImGui::Separator();

                for (int i = 0; i < componentNames.size(); ++i)
                {
                    const std::string& name = componentNames[i];

                    // Filter text
                    if (!componentFilter.PassFilter(name.c_str()))
                        continue;

                    bool alreadyHas =
                        m_ecs.GetEntitySignature(entityID)
                        .test(m_ecs.GetComponentKey(name));

                    if (alreadyHas)
                        ImGui::BeginDisabled();

                    if (ImGui::Selectable(name.c_str()))
                    {
                        auto& action = m_ecs.componentAction.at(name);
                        action->AddComponent(entityID);
                        ImGui::CloseCurrentPopup();
                    }

                    if (alreadyHas)
                        ImGui::EndDisabled();
                }

                ImGui::EndPopup();
            }

            ImGui::SeparatorText("Components");

            if (EntitySignature.test(m_ecs.GetComponentKey(ecs::NameComponent::classname()))) {
                // Retrieve the TransformComponent
                ecs::NameComponent* nc = m_ecs.GetComponent<ecs::NameComponent>(entityID);
                //Display Position
                ImGui::AlignTextToFramePadding();  // Aligns text to the same baseline as the slider
                ImGui::Text("Object Name: ");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(100.0f);
                if (nc->isPrefab) {
                    ImGui::TextDisabled(nc->entityName.c_str());
                }
                else {
                    ImGui::InputText("##NAMETEXT##", &nc->entityName);
                }
                ImGui::SameLine();

                bool hidden = nc->hide;
                if (ImGui::Checkbox("Hide", &hidden)) {
                    for (auto id : m_selectedEntities) {
                        m_commandHistory.AddCommand<CommandHistory::SetGameObjectActive>(id);
                        m_ecs.SetActive(id, !hidden);
                    }
                }

                ImGui::SameLine();
                bool isStatic = nc->isStatic;
                if (ImGui::Checkbox("Static", &isStatic)) {
                    for (auto id : m_selectedEntities) {
                        auto nameComponent = m_ecs.GetComponent<NameComponent>(id);
                        nameComponent->isStatic = isStatic;
                    }
                }

                ImGui::TextDisabled(std::string("Entity ID: " + std::to_string(entityID)).c_str());
                if (!nc->entityGUID.Empty()) {
                    ImGui::SameLine();
                    ImGui::TextDisabled(std::string("GUID: " + nc->entityGUID.GetToString()).c_str());
                }


                {
                    //layer selector
                    static constexpr auto enumNames = magic_enum::enum_names<layer::LAYERS>();

                    // Persistent storage for null-terminated strings
                    static std::array<std::string, enumNames.size()> namesStr{};
                    static std::array<const char*, enumNames.size()> items{};

                    static bool initialized = false;
                    if (!initialized) {
                        for (size_t i = 0; i < enumNames.size(); i++) {
                            namesStr[i] = std::string(enumNames[i]);  // store the string
                            items[i] = namesStr[i].c_str();           // pointer is valid now
                        }
                        initialized = true;
                    }

                    int layer_current = nc->Layer;
                    if (ImGui::Combo("Layers", &layer_current, items.data(), static_cast<int>(items.size()))) {

                        for (auto id : m_selectedEntities) {
                            auto nameComponent = m_ecs.GetComponent<NameComponent>(id);
                            m_layerManager.m_SwapEntityLayer((layer::LAYERS)layer_current, nc->Layer, id);
                        }
                        
                    }
                }

                {
                    // Convert vector to array of char* for ImGui
                    std::vector<const char*> tag_Names(m_tags.size());
                    std::transform(m_tags.begin(), m_tags.end(), tag_Names.begin(), [](const std::string& tag) {  return tag.c_str(); });

                    int item{};
                    const auto& it = std::find(tag_Names.begin(), tag_Names.end(), nc->entityTag);
                    if (it != tag_Names.end()) {
                        item = static_cast<int>(std::distance(tag_Names.begin(), it));
                    }
                    else {
                        tag_Names.push_back(nc->entityTag.c_str());
                        const auto& it2 = std::find(tag_Names.begin(), tag_Names.end(), nc->entityTag);
                        item = static_cast<int>(std::distance(tag_Names.begin(), it2));
                    }

                    if (ImGui::Combo("Tag", &item, tag_Names.data(), static_cast<int>(tag_Names.size()))) {
                        for (auto id : m_selectedEntities) {
                            auto nameComponent = m_ecs.GetComponent<NameComponent>(id);
                            nameComponent->entityTag = m_tags[item];
                        }
                    }
                }

                {
                    // Prefab Overwriting
                    if (nc->isPrefab && !m_prefabSceneMode) {
                        auto* tc = m_ecs.GetComponent<ecs::TransformComponent>(entityID);
                        if (!tc->m_haveParent || !m_ecs.GetComponent<ecs::NameComponent>(tc->m_parentID)->isPrefab) {
                            static bool isHeaderOpen = false;
                            // Instanced ID, <Prefab ID, Comp Sig>
                            static std::map<EntityID, std::pair<EntityID, ecs::ComponentSignature>> comparedResult;

                            bool open = false;
                            std::string headerName = nc->prefabName + " [Changed]";
                            int IMGUI_ID = 0;
                            float pos = ImGui::GetCursorPosX() + ImGui::GetWindowSize().x - 200;

                            if (open = ImGui::BeginCombo("##PrefabChanges", headerName.c_str(), ImGuiComboFlags_HeightLargest)) {

                                DrawEntityChanges(comparedResult, entityID, entityID);

                                // Overwrite All
                                if (ImGui::Button("Overwrite Prefab [All]", { ImGui::GetContentRegionAvail().x, 0 })) {
                                    try {
                                        m_prefabManager.SetPrefabStatus(m_lastClickedEntityId, true);
                                        m_prefabManager.OverwriteScenePrefab(m_lastClickedEntityId);
                                        m_prefabManager.UpdateAllPrefab(nc->prefabName);
                                        // Disable scene to disable all prefab components (eg. Mesh Component & Particle Component)
                                        m_sceneManager.SetSceneActive(nc->prefabName, false);
                                    }
                                    catch (...) {
                                        LOGGING_ERROR("Prefab overwrite, failed");
                                    }
                                    ImGui::EndCombo();
                                    ImGui::End();
                                    return;
                                }

                                if (ImGui::Button("Revert to Prefab [All]", { ImGui::GetContentRegionAvail().x, 0 })) {
                                    try {
                                        m_prefabManager.UpdateAllPrefab(nc->prefabName);
                                    }
                                    catch (...) {
                                        LOGGING_ERROR("Prefab revert, failed");
                                    }
                                    ImGui::EndCombo();
                                    ImGui::End();
                                    return;
                                }

                                if (ImGui::Button("Update All Prefabs in Scene", { ImGui::GetContentRegionAvail().x, 0 })) {
                                    std::vector<std::string> allPrefabsNamesInScene;
                                    auto& sceneData = m_ecs.GetSceneData(m_activeScene);
                                    for (auto id : sceneData.sceneIDs) {
                                        auto iter = std::find(allPrefabsNamesInScene.begin(), allPrefabsNamesInScene.end(), m_ecs.GetComponent<NameComponent>(id)->prefabName);
                                        if (iter == allPrefabsNamesInScene.end()) {
                                            allPrefabsNamesInScene.push_back(m_ecs.GetComponent<NameComponent>(id)->prefabName);
                                        }
                                    }

                                    if (allPrefabsNamesInScene.size() > 0) {
                                        for (auto& sceneName : allPrefabsNamesInScene) {
                                            if (sceneName == "Game Package.prefab" || sceneName == "LockRoomPrefab.prefab") continue;
                                            m_prefabManager.UpdateAllPrefab(sceneName);
                                        }
                                    }
                                }
                                ImGui::EndCombo();
                            }

                            if (isHeaderOpen != open) { // Needed to show change in state
                                if (open) {
                                    m_prefabManager.CompareAll(comparedResult, entityID);
                                }
                                else {
                                    comparedResult.clear();
                                }
                                isHeaderOpen = open;
                            }
                        }
                    }
                }
            }

            const auto& componentKey = m_ecs.GetComponentKeyData();
            int ImguiID = 0;
            for (auto it = componentKey.rbegin(); it != componentKey.rend(); ++it) {
                const auto& ComponentName = it->first;
                auto key = it->second;

                if (EntitySignature.test(key) && ComponentName != ecs::NameComponent::classname()) {
                    auto* component = m_ecs.GetIComponent<ecs::Component*>(ComponentName, entityID);

                    //ImGui::PushID(ImguiID++);
                    //if (componentDrawers.find(ComponentName) != componentDrawers.end()) {
                    //    auto& editorAction = componentDrawers[ComponentName];
                    //    editorAction->Draw(component);
                    //}
                    //else {
                    //    // auto& actionMap = GetComponentActionMap();
                    //     //scrpt components
                    //    DrawFieldComponent(component, ComponentName, entityID);

                    //}
                    //ImGui::PopID();

                    ImGui::PushID(ImguiID++);

                    if (ComponentName == ecs::AudioComponent::classname()) {
                        DrawFieldComponent(component, ComponentName, entityID);
                    }
                    else if (componentDrawers.find(ComponentName) != componentDrawers.end()) {
                        auto& editorAction = componentDrawers[ComponentName];
                        editorAction->Draw(component);
                    }
                    else {
                        DrawFieldComponent(component, ComponentName, entityID);
                    }

                    ImGui::PopID();


                }
            }

            //draw invinsible box - currently not doing anything right now
            //if (ImGui::GetContentRegionAvail().x > 0 && ImGui::GetContentRegionAvail().y > 0) {
            //    ImGui::InvisibleButton("##Invinsible", ImVec2{ ImGui::GetContentRegionAvail().x,ImGui::GetContentRegionAvail().y });
            //    if (ImGui::BeginDragDropTarget())
            //    {
            //        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
            //        {
            //            IM_ASSERT(payload->DataSize == sizeof(std::filesystem::path));
            //            std::filesystem::path* filename = static_cast<std::filesystem::path*>(payload->Data);
            //        }
            //        ImGui::EndDragDropTarget();
            //    }
            //}
        }
    }

 
    ImGui::End();
}   


void gui::ImGuiHandler::DrawFieldComponent(ecs::Component* component, const std::string& ComponentName, ecs::EntityID entityID) {
    // Handle AudioComponent separately
    if (ComponentName == ecs::AudioComponent::classname()) {
        auto* audioComp = static_cast<ecs::AudioComponent*>(component);

        bool open = ImGui::CollapsingHeader(ComponentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Delete Component")) {
                m_ecs.componentAction.at(ComponentName)->RemoveComponent(entityID);
            }
            if (ImGui::MenuItem("Reset Component")) {
                m_ecs.componentAction.at(ComponentName)->ResetComponent(entityID);
            }
            ImGui::EndPopup();
        }

        if (open) {
            auto& audioFiles = audioComp->audioFiles;

            ImGui::SeparatorText("Audio Files");

            if (ImGui::Button("+ Add Audio File")) {
                audioFiles.emplace_back();
            }

            ImGui::Spacing();

            int removeIndex = -1;

            for (int i = 0; i < static_cast<int>(audioFiles.size()); ++i) {
                auto& af = audioFiles[i];

                ImGui::PushID(i);

                std::string label;

                if (!af.name.empty()) {
                    // Manually set name
                    label = af.name;
                }
                else if (!af.audioGUID.Empty()) {
                    // if not just use filename
                    std::filesystem::path audioPath = m_assetManager.GetFileFromGUID(af.audioGUID);
                    if (!audioPath.empty()) {
                        label = audioPath.filename().string(); // e.g. "explosion.wav"
                    }
                    else {
                        label = "Audio File [" + std::to_string(i) + "]";
                    }
                }
                else {
                    // No GUID assigned yet
                    label = "Audio File [" + std::to_string(i) + "]";
                }

                std::string fullLabel = label + "##AudioFile_" + std::to_string(i);
                bool entryOpen = ImGui::CollapsingHeader(fullLabel.c_str());


                if (ImGui::BeginPopupContextItem()) {
                    if (ImGui::MenuItem("Remove")) removeIndex = i;
                    ImGui::EndPopup();
                }

                if (entryOpen) {
                    ImGui::Indent();

                    ImGui::InputText("Name", &af.name);

                    ImGui::Text("Audio GUID:");
                    ImGui::SameLine();

                    std::string guidDisplay = af.audioGUID.Empty()
                        ? "[ Drop WAV here ]"
                        : af.audioGUID.GetToString();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.3f, 1.f)); 
                    ImGui::Button(guidDisplay.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                    ImGui::PopStyleColor(2);

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file")) {

                            std::filesystem::path droppedPath(
                                static_cast<const char*>(payload->Data)
                            );

                            if (droppedPath.extension() == ".wav" || droppedPath.extension() == ".WAV") {
                                utility::GUID resolvedGUID = m_assetManager.GetGUIDfromFilePath(droppedPath);
                                if (!resolvedGUID.Empty()) {
                                    af.audioGUID = resolvedGUID;

                                    if (af.name.empty()) {
                                        af.name = droppedPath.stem().string();
                                    }
                                }
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }



                    ImGui::Text("Bank GUID:");
                    ImGui::SameLine();

                    std::string bankDisplay = af.audioBankGUID.Empty()
                        ? "[ Drop Bank here ]"
                        : af.audioBankGUID.GetToString();

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.5f, 1.f)); // blue tint on hover
                    ImGui::Button(bankDisplay.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 0));
                    ImGui::PopStyleColor(2);

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file")) {

                            std::filesystem::path droppedPath(
                                static_cast<const char*>(payload->Data)
                            );

                            utility::GUID resolvedGUID = m_assetManager.GetGUIDfromFilePath(droppedPath);
                            if (!resolvedGUID.Empty()) {
                                af.audioBankGUID = resolvedGUID;
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }



                    ImGui::InputText("Studio Event Path", &af.studioEventPath);

                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::SliderFloat("Volume", &af.volume, 0.0f, 1.0f);
                    ImGui::SliderFloat("Pan", &af.pan, -1.0f, 1.0f);

                    ImGui::Spacing();
                    ImGui::Separator();

                    ImGui::Columns(2, "AudioFlagsColumns", false);
                    ImGui::Checkbox("Loop", &af.loop);           ImGui::NextColumn();
                    ImGui::Checkbox("Play On Start", &af.playOnStart); ImGui::NextColumn();
                    ImGui::Checkbox("Is BGM", &af.isBGM);        ImGui::NextColumn();
                    ImGui::Checkbox("Is SFX", &af.isSFX);        ImGui::NextColumn();
                    ImGui::Checkbox("Use 3D", &af.use3D);        ImGui::NextColumn();
                    ImGui::Columns(1);

                    if (af.use3D) {
                        ImGui::Indent();
                        ImGui::SliderFloat("Min Distance", &af.minDistance, 0.0f, 50.0f);
                        ImGui::SliderFloat("Max Distance", &af.maxDistance, 0.0f, 200.0f);
                        ImGui::Unindent();
                    }

                    ImGui::Spacing();

                    const char* sourceTypes[] = { "Core", "Studio" };
                    int srcType = static_cast<int>(af.sourceType);
                    if (ImGui::Combo("Source Type", &srcType, sourceTypes, 2)) {
                        af.sourceType = static_cast<AudioSourceType>(srcType);
                    }

                    ImGui::Spacing();

                    ImGui::BeginDisabled();
                    ImGui::Checkbox("Has Played", &af.hasPlayed);
                    ImGui::Checkbox("Request Play", &af.requestPlay);
                    ImGui::EndDisabled();

                    ImGui::Unindent();
                }

                ImGui::PopID();
                ImGui::Spacing();
            }

            if (removeIndex >= 0) {
                audioFiles.erase(audioFiles.begin() + removeIndex);
            }
        }
        return;
    }


    auto fields = m_reflectionField.GetAction();

    if (fields.find(ComponentName) == fields.end()) return;

    auto& action = fields.at(ComponentName);
    bool open = ImGui::CollapsingHeader(ComponentName.c_str(), ImGuiTreeNodeFlags_DefaultOpen);

    if (ImGui::BeginPopupContextItem()) {
        auto action = m_ecs.componentAction.at(ComponentName);
        if (ComponentName != ecs::TransformComponent::classname() && ImGui::MenuItem("Delete Component")) {
            action->RemoveComponent(entityID);
        }
        if (ImGui::MenuItem("Reset Component")) {

            action->ResetComponent(entityID);
        }

        ImGui::EndPopup();
    }

    if (open) {

        auto fieldReference = action->GetFieldReference(component);
        auto names = action->GetNames();
        DrawComponents draw(names);

        for (auto& field : fieldReference) {
            
            if (field.IsType<float>()) {
                draw(field.AsType<float>());
            }
            else if (field.IsType<int>()) {
                draw(field.AsType<int>());
            }
            else if (field.IsType<bool>()) {
                draw(field.AsType<bool>());
            }
            else if (field.IsType<std::string>()) {
                draw(field.AsType<std::string>());
            }
            else if (field.IsType<glm::vec3>()) {
                draw(field.AsType<glm::vec3>());
            }
            else if (field.IsType<glm::vec2>()) {
                draw(field.AsType<glm::vec2>());
            }
            else if (field.IsType<glm::vec4>()) {
                draw(field.AsType<glm::vec4>());
            }
            else if (field.IsType<std::vector<std::string>>()) {
                draw(field.AsType<std::vector<std::string>>());
            }
            else if (field.IsType<std::vector<glm::vec3>>()) {
                draw(field.AsType<std::vector<glm::vec3>>());
            }
            else if (field.IsType<std::vector<glm::vec2>>()) {
                draw(field.AsType<std::vector<glm::vec2>>());
            }
            else if (field.IsType<std::vector<glm::vec4>>()) {
                draw(field.AsType<std::vector<glm::vec4>>());
            }
            else if (field.IsType<utility::GUID>()) {
                draw(field.AsType<utility::GUID>());
            }
            else if (field.IsType<std::vector<utility::GUID>>()) {
                draw(field.AsType<std::vector<utility::GUID>>());
            }

            else {
                draw.count++;
            }
        }
    }
}

void gui::ImGuiHandler::DrawEntityChanges(std::map<EntityID, std::pair<EntityID, ecs::ComponentSignature>>& result, ecs::EntityID entityID, ecs::EntityID root) {
    auto childList = m_ecs.GetChild(entityID);
    ecs::NameComponent* nc = m_ecs.GetComponent<ecs::NameComponent>(entityID);
    if (childList.has_value()) {
        std::string name = nc->entityName;
        if (ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawComponentChanges(result, entityID, root);

            for (auto& child : childList.value()) {
                DrawEntityChanges(result, child, root);
            }
            ImGui::TreePop();
        }
    }
    else {
        if (ImGui::TreeNodeEx(nc->entityName.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_DefaultOpen)) {
            DrawComponentChanges(result, entityID, root);
            ImGui::TreePop();
        }
    }
}

void gui::ImGuiHandler::DrawComponentChanges(std::map<EntityID, std::pair<EntityID, ecs::ComponentSignature>>& result, ecs::EntityID entityID, ecs::EntityID root) {
    int IMGUI_ID = 0;
    ecs::NameComponent* nc = m_ecs.GetComponent<ecs::NameComponent>(entityID);
    if (result.find(entityID) != result.end()) {
        if (result.find(entityID)->second.first < 0) return;

        const auto& componentKey = m_ecs.GetComponentKeyData();
        for (const auto& [ComponentName, key] : componentKey) {
            if (result.find(entityID)->second.second.test(key)) {
                ImGui::Text("-\t%s", ComponentName.c_str());
                ImGui::SameLine();
                ImGui::PushID(IMGUI_ID++);
                if (ImGui::Button("Revert back")) {
                    // Look for component of prefab and set data into me
                    m_prefabManager.RevertToPrefab_Component(entityID, ComponentName, result.find(entityID)->second.first);
                    m_prefabManager.CompareAll(result, root);
                }
                ImGui::SameLine();
                if (ImGui::Button("Overwrite Prefab")) {
                    // Look for component of prefab and set data from me
                    m_prefabManager.OverwritePrefab_Component(entityID, ComponentName, nc->prefabName, result.find(entityID)->second.first);
                    m_prefabManager.CompareAll(result, root);
                }
                ImGui::PopID();
            }
        }
    }
}
