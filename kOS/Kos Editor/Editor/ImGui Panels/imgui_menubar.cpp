/******************************************************************/
/*!
\file      Imgui_menubar.cpp
\author    Ng Jaz winn, jazwinn.ng , 2301502
\par       jazwinn.ng@digipen.edu
\date      Oct 02, 2025
\brief     This file contains the defination of the draw mainmenu bar window
           function.



Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/********************************************************************/

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Editor.h"
#include "imgui_internal.h"
#include "Editor/CommandHistory.h"
#include "Editor/EditorCamera.h"
#include "Scene/SceneManager.h"
#include "AssetManager/AssetManager.h"
#include "Configs/ConfigPath.h"
#include <RAPIDJSON/stringbuffer.h>
#include <RAPIDJSON/prettywriter.h>
namespace menubarHelper {
    void SerializeFloat(std::string name, float data, rapidjson::Value& keys, rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value field;
        field.SetString(name.c_str(), allocator);
        keys.AddMember(field, data, allocator);
    }
    void DeserializeFloat(std::string name, float& data, const rapidjson::Value& keys) {
        if (keys.HasMember(name.c_str()) && keys[name.c_str()].IsFloat()) {
            data = keys[name.c_str()].GetFloat();
        }
    }
}

void gui::ImGuiHandler::DrawMainMenuBar() {
   

    bool openNewFilepopup = false;
    bool createNewPostProcessProfile=false;
    static bool createCameraProfile = false;
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New Scene")) {
                openNewFilepopup = true;
            }
           
            if (ImGui::MenuItem("Save Scene")) {
                m_sceneManager.SaveAllActiveScenes(true);
            }



            if (m_ecs.sceneMap.size() < 0 || ImGui::MenuItem("Open Scene")) {

                openAndLoadSceneDialog();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Preferences", NULL, &openPreferencesTab);
            //Add menu item 4 camera settings
            ImGui::MenuItem("Camera Settings", NULL, &createCameraProfile);

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Profile")) {
            if (ImGui::MenuItem("New Scene"))createNewPostProcessProfile = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Save Layout")) {
                SaveLayout();            
            }
            ImGui::EndMenu();
        }

        if (openNewFilepopup) {
            ImGui::OpenPopup("New Scene");
        }

        if (ImGui::BeginPopupModal("New Scene", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter Scene Name");
            ImGui::Separator();

            //static int unused_i = 0;
            //ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

            static char str1[128] = "";
            ImGui::InputTextWithHint(".json", "Enter scene name here", str1, IM_ARRAYSIZE(str1));

            if (ImGui::Button("Save", ImVec2(120, 0))) { 
                std::string m_jsonFilePath{ m_assetManager.GetAssetManagerDirectory() + "/Scene/" }; //TODO temp open window in future
                std::string scene = m_jsonFilePath + str1 + ".json";
                m_activeScene = scene;
                if (!scene.empty()) {
                    if (m_sceneManager.CreateNewScene(scene)) {
                        //remove all scenes
                        m_sceneManager.ClearAllScene();
                        m_sceneManager.LoadScene(scene);
                        m_activeScene = std::string(str1) + ".json";
                        m_lastClickedEntityId = -1;
                    }
                    else {
                        LOGGING_ERROR("Fail to create scene");
                    }
                }
                else {
                    LOGGING_WARN("Please Insert a Scene Name");
                }

                ImGui::CloseCurrentPopup(); 
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

        if (createCameraProfile) {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y * 0.8f), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Camera Settings", &createCameraProfile, ImGuiWindowFlags_NoDocking)) {
                //Set camera sensitivity
                float sensUpscale = EditorCamera::editorCamera.sens*100.f;
                float orbitSensUpscale = EditorCamera::editorCamera.orbitSens * 100.f;
                ImGui::SliderFloat("Sensitivity", &sensUpscale, 0.0f, 100.0f);
                ImGui::SliderFloat("Orbit Sensitivity", &orbitSensUpscale, 0.0f, 100.0f);
                EditorCamera::editorCamera.sens = sensUpscale / 100.f;
                EditorCamera::editorCamera.orbitSens = orbitSensUpscale / 100.f;

                //Create button to save
                if(ImGui::Button("Save")) {
                    //Save settings in a json file
                    rapidjson::Document doc;
                    if (!doc.IsArray()) {
                        doc.SetArray();  // Initialize as an empty array
                    }
                    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                    rapidjson::Value data(rapidjson::kObjectType);
                    menubarHelper::SerializeFloat("Sensitivity",EditorCamera::editorCamera.sens, data, allocator);
                    menubarHelper::SerializeFloat("Orbit sensitivity", EditorCamera::editorCamera.orbitSens, data, allocator);
                    doc.PushBack(data, allocator);
                    rapidjson::StringBuffer writeBuffer;
                    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(writeBuffer);
                    doc.Accept(writer);
                    std::ofstream file(configpath::cameraSettingPath);
                    if (!file.is_open()) {
                        LOGGING_WARN("Unable to Save Style setting");
                        file.close();
                        return;
                    }
                    else {
                        file << writeBuffer.GetString();
                        file.close();
                        LOGGING_DEBUG("Setting Saved");
                    }
                }
                ImGui::End();
            }
        }
        if (createNewPostProcessProfile) {
            ImGui::OpenPopup("New Profile");
        }

        if (ImGui::BeginPopupModal("New Profile", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Enter Profile name here");
            ImGui::Separator();

            //static int unused_i = 0;
            //ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

            static char str1[128] = "";
            ImGui::InputTextWithHint(".prof", "Enter Profile name here", str1, IM_ARRAYSIZE(str1));

            if (ImGui::Button("Save", ImVec2(120, 0))) {
                if (strlen(str1) == 0) {
                    LOGGING_WARN("Please insert a profile name");
                }
                else
                {
                    std::string basePath = m_assetManager.GetAssetManagerDirectory() + "/PostProcessProfile/";
                    std::string profilePath = basePath + std::string(str1) + ".prof";

                    // Create JSON document
                    rapidjson::Document doc;
                    doc.SetObject();
                    auto& allocator = doc.GetAllocator();

                    // ProfileName
                    rapidjson::Value profileName;
                    profileName.SetString(str1, allocator);
                    doc.AddMember("ProfileName", profileName, allocator);

                    // Empty effects array
                    rapidjson::Value postProfileEffects(rapidjson::kArrayType);
                    doc.AddMember("PostProfileEffects", postProfileEffects, allocator);

                    // Write JSON to file
                    rapidjson::StringBuffer buffer;
                    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                    doc.Accept(writer);

                    std::ofstream ofs(profilePath, std::ios::out | std::ios::trunc);
                    if (ofs.is_open()) {
                        ofs << buffer.GetString();
                        ofs.close();
                    }
                    else {
                        LOGGING_ERROR("Failed to create profile file");
                    }
                }

                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
            ImGui::EndPopup();
        }

		static bool reloadingscript = false;
        static int result = 1;
        if (ImGui::BeginMenu("Hot Reload")) {
            if (ImGui::MenuItem("Reload Script")) {

                std::thread hotReloadThread([&]() {
                    std::string scriptPath = configpath::scriptReloadFilePath;
                    std::string tempBatch = "tempHotReload.bat";

                    std::ofstream batch(tempBatch);
                    batch << "@echo off\n";
                    batch << "cd /d \"" << std::filesystem::path(scriptPath).parent_path().string() << "\"\n";
                    batch << "call \"" << std::filesystem::path(scriptPath).filename().string() << "\"\n";
                    batch.close();

                    std::string command = "\"" + tempBatch + "\"";

                    // Run batch (blocks this thread)
                    int count = 0;
                    
                    do {
                        result = std::system(command.c_str());
                    } while ((result != 0) && (count++ < 1));
                    // this so scuff at this point im done with life


                    if(result != 0) {
                        LOGGING_ERROR("Hot reload script failed with exit code: {}", result);
                    } else {
                        LOGGING_INFO("Hot reload script executed successfully.");

					}

                    std::filesystem::remove("tempHotReload.bat");

					reloadingscript = false;
                    });

                hotReloadThread.detach();
				reloadingscript = true;

            }
            ImGui::EndMenu();
        }


        ImGui::EndMainMenuBar();

        if (result == 0) {
            // If script executed successfully, perform hot-reload in the main thread
            ScriptHotReload();
			result = 1; // Reset result to avoid repeated reloads
        }

        if (reloadingscript) {

            // Draw last for topmost layering
            ImGui::SetNextWindowBgAlpha(0.9f); // slightly transparent
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(180, 60), ImGuiCond_Always);

            // Make it non-interactive and floating above others
            ImGui::Begin("##CompilingOverlay", nullptr,
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoInputs |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoDocking);

            ImGui::Text("Compiling...");
            ImGui::SameLine();

            // Simple rotating spinner
            ImVec2 pos = ImGui::GetCursorScreenPos();
            float radius = 8.0f;
            float thickness = 3.0f;
            float time = (float)ImGui::GetTime();
            int num_segments = 30;
            float start = fabsf(sinf(time * 1.8f)) * (num_segments - 5);
            const float a_min = IM_PI * 2.0f * start / num_segments;
            const float a_max = IM_PI * 2.0f * (num_segments - 3) / num_segments;
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            draw_list->PathClear();

            for (int i = 0; i < num_segments; i++)
            {
                float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
                draw_list->PathLineTo(ImVec2(pos.x + radius + cosf(a) * radius,
                    pos.y + radius + sinf(a) * radius));
            }

            draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), false, thickness);

            ImGui::End();
        }
    }  
}

 void gui::ImGuiHandler::ScriptHotReload() {

    m_sceneManager.ImmediateClearScene(CACHEDSCENE);
    m_sceneManager.SaveAllActiveScenes();

    auto scenelist = m_sceneManager.GetAllScenesPath();

    //CLEAR ALL SCENES BEFORE RELOADING DLL, ELSE CRASH
    m_sceneManager.ClearAllSceneImmediate();



    //Unload the DLL
    m_scriptManager.UnloadDLL();


    Sleep(1000);

    std::filesystem::path source = std::filesystem::absolute(configpath::scriptWatherFilePath);

    // Get parent directory of the current folder (up one level)
    std::filesystem::path targetDir = source.parent_path().parent_path(); // moves up one directory
    std::filesystem::path target = targetDir / "script.dll";

    try {
        std::filesystem::copy_file(source, target, std::filesystem::copy_options::overwrite_existing);
        std::cout << "Copied to: " << target << std::endl;
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error copying file: " << e.what() << std::endl;
    }


    //load the DLL
    m_scriptManager.RunDLL();

    //load back scene
    m_prefabManager.LoadAllPrefabs();
    for (const auto& scenepath : scenelist) {
        if (scenepath.path.extension().string() != ".prefab") {
            m_sceneManager.ImmediateLoadScene(scenepath.path);
            auto& sceneData = m_ecs.GetSceneData(scenepath.path.filename().string());
            sceneData.isActive = scenepath.isActive;
        }
    }


    //set back command history
    m_commandHistory.Init();

}
 void  gui::ImGuiHandler::DeserializeCameraSetting() {
     std::ifstream file(configpath::cameraSettingPath);
     if (!file.is_open()) {
         file.close();
         //Use default settings 
         return;;
     }
     std::string fileContent((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
     file.close();

     rapidjson::Document doc;
     doc.Parse(fileContent.c_str());
     ImGuiStyle& style = ImGui::GetStyle();
     menubarHelper::DeserializeFloat("Sensitivity", EditorCamera::editorCamera.sens, doc[0]);
     menubarHelper::DeserializeFloat("Orbit sensitivity", EditorCamera::editorCamera.orbitSens, doc[0]);

 }

