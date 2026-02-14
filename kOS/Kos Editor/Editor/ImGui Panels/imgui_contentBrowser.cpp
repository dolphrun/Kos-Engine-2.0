/******************************************************************/
/*!
\file      imgui_contentBrowser.cpp
\author    Ng Jaz Winn, jazwinn.ng, 2301502
\par       jazwinn.ng@digipen.edu
\date      Sep 30, 2025
\brief     

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
*/
/******************************************************************/


#include "Editor.h"

#include <filesystem>
#include <string>

#include <imgui_internal.h>

#include "AssetManager/AssetManager.h"
#include "Scene/SceneManager.h"
#include "Application/ApplicationData.h"
#include "Inputs/Input.h"
#include "Configs/ConfigPath.h"
#include "AssetManager/Prefab.h"


namespace gui {
	static std::string searchString;
	static float padding = 20.f;
	static float thumbnail = 100.f;

	void MoveFolder(const std::filesystem::path& newDirectory) {
		if (ImGui::BeginDragDropTarget())
		{
			//assetmanager::AssetManager* assetmanager = assetmanager::AssetManager::m_funcGetInstance();
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
			{

				IM_ASSERT(payload->DataSize == sizeof(std::filesystem::path));
				std::filesystem::path* filename = static_cast<std::filesystem::path*>(payload->Data);


				if (newDirectory == *filename) return;
				std::filesystem::path destinationFile = newDirectory / filename->filename();

				std::filesystem::rename(*filename, destinationFile);

			}
			ImGui::EndDragDropTarget();
		}
	}

	bool ImGuiHandler::ImageButton(const std::filesystem::path& directoryString) {

		std::string fileName = directoryString.filename().string();
		ImVec2 buttonSize = { thumbnail, thumbnail };
		bool isClicked = false;
		unsigned int textureID = 0;

		if (std::filesystem::is_directory(directoryString)) {
			textureID = m_assetManager.folderTexture->GetTextureID();
		}
		else {
			textureID = m_assetManager.fileTexture->GetTextureID();
		}




		if (textureID != 0) {
			// Render the Image Button
			// We cast the textureID just like we did for ImGui::Image()
			// Modern ImGui requires a string ID as the first argument for ImageButton
			isClicked = ImGui::ImageButton(
				fileName.c_str(),        // Unique ID so ImGui can track clicks
				(void*)(intptr_t)textureID,     // The texture to render
				buttonSize                      // Size of the button
			);
		}
		else {
			// Render the standard Text Button
			isClicked = ImGui::Button(fileName.c_str(), buttonSize);
		}

		return isClicked; // Returns true on the exact frame the user clicks the button
	}

	void ImGuiHandler::DrawContentBrowser() {

		static std::filesystem::path assetDirectory = m_assetManager.GetAssetManagerDirectory();
		static std::filesystem::path currentDirectory = assetDirectory;

		if (ImGui::Begin("Content Browser")) {
			if (ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Border)) {
				static bool isSelected{ false };
				for (auto& directoryPath : std::filesystem::directory_iterator(assetDirectory)) {
					std::string directoryString = directoryPath.path().filename().string();
					isSelected = ImGui::Selectable(directoryString.c_str());
					MoveFolder(assetDirectory / directoryPath.path().filename());

					if (std::filesystem::is_directory(directoryPath) && isSelected) {
						currentDirectory = assetDirectory / directoryPath.path().filename();
						//reset search string
						searchString.clear();
					}
				}
				ImGui::EndChild();
			}

			ImGui::SameLine();

			if (ImGui::BeginChild("ChildLa", ImVec2(0, ImGui::GetContentRegionAvail().y), 0, ImGuiWindowFlags_MenuBar)) {
				if (ImGui::BeginPopupContextWindow()) {
					if (ImGui::MenuItem("Add Folder")) {
						std::string path = "/New Folder";

						int count{ 1 };
						while (std::filesystem::exists(currentDirectory.string() + path)) {
							path = "/New Folder_" + std::to_string(count++);
						}

						if (std::filesystem::create_directories(currentDirectory.string() + path)) {

							LOGGING_INFO("Directory created successfully!");
						}
						else {
							LOGGING_ERROR("Directory already exists or failed to create!");
						}

					}
					if (ImGui::MenuItem("Create post process profile")) {
						////Create object at current directory
						//std::string fileName = materialData.materialName + ".prof";
						//std::string filepath = currentDirectory.string() + fileName;
						//PostPro
						//serialization::WriteJsonFile(filepath, &materialData.data);
						// std::cout << "TEST CREATION\n";
						LOGGING_POPUP("Material Successfully Added");

					}
					if (ImGui::MenuItem("Reload Browser")) {
						//TODO: reload browser
					}
					ImGui::EndPopup();
				}

				//search bar
				if (m_prefabSceneMode)searchString.clear();

				//menu bar for search
				if (ImGui::BeginMenuBar()) {
					ImGui::Text("Search:");
					ImGui::SameLine(); // Keep the next widget on the same line
					ImGui::SetNextItemWidth(300);
					if (ImGui::InputText("##4312Search", &searchString)) {}
					ImGui::EndMenuBar(); // End menu bar
				}

				//back button		
				if (currentDirectory != assetDirectory) {
					bool open = ImGui::Button("Back");
					MoveFolder(currentDirectory.parent_path());
					if (open) {
						currentDirectory = currentDirectory.parent_path();
						searchString.clear();
					}

				}
				else {
					ImGui::NewLine();
				}

				float cellsize = padding + thumbnail;
				float panelwidth = ImGui::GetContentRegionAvail().x;
				int columns = (int)(panelwidth / cellsize);
				if (columns <= 0) {
					columns = 1;
				}
				ImGui::Columns(columns, 0, false);

				// Render Icons (Folder or Files)
				for (auto& directoryPath : std::filesystem::directory_iterator(currentDirectory)) {
					std::string directoryString = directoryPath.path().filename().string();
					

					//skip if file is not same as search and skip .meta files
					if (!searchString.empty() && !containsSubstring(directoryString, searchString) ||
						directoryPath.path().filename().extension().string() == ".meta") {
						continue;
					}

					ImageButton(directoryPath.path());

					if (directoryPath.is_directory()) {
						// if a folder
						MoveFolder(currentDirectory / directoryPath.path().filename());


						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
							currentDirectory /= directoryPath.path().filename();
							//reset search
							searchString.clear();
						}
					}
					else {
						//case for prefabs and scene
						if (directoryPath.path().filename().extension().string() == ".prefab") {

							if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
								//skip if active scene is filename
								if (m_activeScene == directoryPath.path().filename())continue;

								const auto& prefabscene = m_ecs.sceneMap.find(directoryPath.path().filename().string());
								if (prefabscene == m_ecs.sceneMap.end()) {
									LOGGING_ERROR("Prefab not loaded");
									continue;
								}

								//skip if prefab mode alraedy true
								if (!m_prefabSceneMode) {
									m_savedSceneState.clear();
									for (auto& scene : m_ecs.sceneMap) {
										if (scene.second.isPrefab == false) {
											//save all scenes active state
											m_savedSceneState[scene.first] = scene.second.isActive;
										}
									}

								}

								// clear save scene state

								// unload all regular scenes
								for (auto& [scene, sceneData] : m_ecs.sceneMap) {

									m_sceneManager.SetSceneActive(scene, false);
								}

								// set prefab to active
								m_sceneManager.SetSceneActive(prefabscene->first, true);

								// Duplicate Entity and add it into original scene. Will be removed when m_prefabSceneMode is set back to false. 
								// (Duped Entity used to check if any edits has been made to prefab)
								//duppedID = ecs::ECS::GetInstance()->DuplicateEntity(prefabscene->second.prefabID, m_activeScene);
								//ecs::ECS::GetInstance()->GetComponent<ecs::NameComponent>(duppedID)->prefabName = prefabscene->first;

								//set prefab as active scene
								m_activeScene = directoryPath.path().filename().string();

								m_prefabSceneMode = true;

								m_clickedEntityId = -1;
							}
						}
						else if (directoryPath.path().filename().extension().string() == ".json") {

							if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {

								m_sceneManager.ClearAllScene();
								m_sceneManager.LoadScene(directoryPath.path());
								if (!m_prefabSceneMode) {
									m_activeScene = directoryPath.path().filename().string();
								}
								else {
									m_ecs.sceneMap.find(directoryPath.path().filename().string())->second.isActive = false;
									m_savedSceneState[directoryPath.path().filename().string()] = true;
								}

								m_clickedEntityId = -1;

							}
						}
						else {

							if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
								std::filesystem::path metaPath = directoryPath.path().string() + ".meta";
								if (std::filesystem::exists(metaPath)) {
									selectedAsset = serialization::ReadJsonFile<AssetData>(metaPath.string());
									AssetPath = metaPath;
								}
							}
						}
					}

					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
						AssetPayload data;
						data.GUID = m_assetManager.GetGUIDfromFilePath(directoryPath.path());
						std::string filepath = directoryPath.path().string();
						
						std::strncpy(data.path, filepath.c_str(), sizeof(data.path) - 1);

						ImGui::SetDragDropPayload("file", &data, sizeof(data));

						if (!data.GUID.Empty()) {
							ImGui::Text(data.GUID.GetToString().c_str());
						}	
						else {
							ImGui::Text(filepath.c_str());
						}
						ImGui::EndDragDropSource();
					}

					//create context window
					static bool rename = false;
					static std::filesystem::path selectedfile{};
					if (ImGui::BeginPopupContextItem()) {
						if (ImGui::MenuItem("Rename")) {
							rename = true;
							selectedfile = directoryString;
						}
						if (ImGui::MenuItem("Delete")) {
							std::filesystem::remove_all(directoryPath.path());
						}
						if (!directoryPath.is_directory()) {

							//if (ImGui::BeginMenu("Compile"))
							//{
							//	if (ImGui::MenuItem("Compile"))
							//	{
							//		assetmanager->Compilefile(directoryPath);
							//	}
							//	ImGui::EndMenu();
							//}
							const auto& compilerMap = m_assetManager.GetCompilerMap();

						if(compilerMap.find(directoryPath.path().filename().extension().string()) != compilerMap.end())
						{
							if (ImGui::BeginMenu("Reload As"))
							{
								for(const auto& comp : compilerMap.at(directoryPath.path().filename().extension().string()))
								{
									if (ImGui::MenuItem(comp.type.c_str()))
									{
										m_assetManager.Compilefile(directoryPath.path());
									}
								}
								ImGui::EndMenu();
							}
						}


					}
					if (directoryPath.path().filename().extension().string() == ".prof" && ImGui::MenuItem("Set as current scene profile")) {
						//Load profile
						std::filesystem::path metaPath = directoryPath.path().string() + ".meta";
						AssetData assetData = serialization::ReadJsonFile<AssetData>(metaPath.string());
							//std::cout<< m_activeScene <<'\n';
							const auto& scene = m_ecs.sceneMap.find(m_activeScene);
							scene->second.postProcessingProfile= assetData.GUID;
							//Set post processing in Graphics manager
							m_resourceManager.GetResource<R_PostProcessingProfile>(assetData.GUID)->profile;
							m_graphicsManager.postProcessProfile = &m_resourceManager.GetResource<R_PostProcessingProfile>(assetData.GUID)->profile;
						//scenes .startScene = assetData.GUID;
					}
					if (directoryPath.path().filename().extension().string() == ".json" && ImGui::MenuItem("Set as Startup Scene")) {


							std::filesystem::path metaPath = directoryPath.path().string() + ".meta";

							if (!std::filesystem::exists(metaPath)) {
								LOGGING_POPUP("Meta file does not exist, Compile First");
							}

							WindowSettings data = serialization::ReadJsonFile<WindowSettings>(configpath::gameConfigFilePath);
							AssetData assetData = serialization::ReadJsonFile<AssetData>(metaPath.string());

							data.startScene = assetData.GUID;

							serialization::WriteJsonFile<WindowSettings>(configpath::gameConfigFilePath, &data, true);
						}


						ImGui::EndPopup();
					}


					if (rename && (selectedfile.string() == directoryString)) {
						if (ImGui::InputText("##rename", m_charBuffer, IM_ARRAYSIZE(m_charBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
							//TODO check if file has extension, keep the extension
							std::string extension{};
							if (!directoryPath.is_directory()) {
								extension = directoryPath.path().filename().extension().string();
								ImGui::SameLine();
								ImGui::Text(extension.c_str());
							}

							std::filesystem::path path = std::filesystem::current_path();
							std::string newpath = path.string() + "\\" + currentDirectory.string() + "\\" + m_charBuffer + extension;
							std::string oldpath = path.string() + "\\" + currentDirectory.string() + "\\" + directoryString;


							LOGGING_INFO("RENAME WIP");
							//assetmanager->m_RenameAsset(oldpath, newpath);

							rename = false;
							selectedfile.clear();

							//TODO edge cases,
							//Update assets if any of them are renamed
						}
					}
					else {
						float cellWidth = thumbnail;

						ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cellWidth);

						ImGui::SetWindowFontScale(0.8f);
						ImGui::TextWrapped("%s",
							directoryPath.path().filename().stem().string().c_str());
						ImGui::SetWindowFontScale(1.f);

						ImGui::PopTextWrapPos();
					}
					ImGui::NextColumn();
				}

				//allow drag and drop on child directory
				if (ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows)) {
					if (!m_input.droppedFiles.empty()) {
						for (const auto& file : m_input.droppedFiles) {
							std::filesystem::path source = file;
							std::filesystem::path destination = currentDirectory;

							if (!std::filesystem::exists(source)) {
								LOGGING_WARN("Source directory does not exist ");
							}
							else {
								if (std::filesystem::is_directory(source)) {
									destination /= source.filename();
									if (std::filesystem::exists(destination)) {
										LOGGING_WARN("Directory already exists");
										continue;
									}
								}

								if (std::filesystem::exists(destination / source.filename())) {
									LOGGING_WARN("Directory already in folder");
									continue;
								}

								// Copy directory and all contents recursively
								std::filesystem::copy(source, destination, std::filesystem::copy_options::recursive);
							}

						}
						// Clear the vector if you want to reset after displaying
						m_input.droppedFiles.clear();
					}
				}
				ImGui::EndChild();
			}

			// Drag and Drop Interaction for Creating New Prefabs
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
				{
					IM_ASSERT(payload->DataSize == sizeof(ecs::EntityID));
					LOGGING_DEBUG("Dropping To Save Prefab");
					ecs::EntityID id = *static_cast<ecs::EntityID*>(payload->Data);
					m_prefabManager.m_SaveEntitytoPrefab(id);
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();
	}
}