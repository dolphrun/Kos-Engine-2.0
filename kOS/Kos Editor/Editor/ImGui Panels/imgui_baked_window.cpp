#include "Editor.h"
#include "config/pch.h"
#include "AssetManager/AssetManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/GraphicsManager.h"
std::string GenerateRandomGUIDBaked() {
	static std::random_device dev;
	static std::mt19937 rng(dev());

	std::uniform_int_distribution<int> dist(0, 15);

	const char* v = "0123456789abcdef";
	const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

	std::string res;
	for (int i = 0; i < 16; i++) {
		if (dash[i]) res += "-";
		res += v[dist(rng)];
		res += v[dist(rng)];
	}
	return res;
}
void gui::ImGuiHandler::DrawBakedWindow() {

	if (ImGui::Begin("Baked Lighting")) {

		//Get list og GO tags

		static int selected = 0;
		static std::vector<std::string> addedTags;

		// Dropdown to select what to add
		ImGui::Text("Add Item:");
		ImGui::SameLine();
		if (ImGui::BeginCombo("##itemSelect", m_tags[selected].c_str()))
		{
			for (int i = 0; i < m_tags.size(); i++)
			{
				bool isSelected = (selected == i);
				if (ImGui::Selectable(m_tags[i].c_str(), isSelected))
					selected = i;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		if (ImGui::Button("Add"))
		{
			addedTags.push_back(m_tags[selected]);
		}
		//Display list of tags
		ImGui::Separator();
		ImGui::Text("Tags:");
		if (!addedTags.size())ImGui::Text("All");
		for (int i = 0; i < addedTags.size(); i++)
		{
			// Name
			ImGui::Text("%s", addedTags[i].c_str());
			ImGui::SameLine();

			// Push to the right side (optional)
			// ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 80);

			// Remove button
			if (ImGui::SmallButton(("Remove##" + std::to_string(i)).c_str()))
			{
				addedTags.erase(addedTags.begin() + i);
				i--; // adjust index after erase
				continue;
			}
		}
		if (ImGui::Button("Bake Lights"))
		{

			LOGGING_INFO("It's a piece of cake to bake a pretty cake");
			//Just bake first light first
			//PointLightData* pld;
			int i = 0;

			auto sceneData = m_ecs.GetSceneData(m_ecs.GetSceneByEntityID(m_lastClickedEntityId));
			//Get all mesh data with tags
			std::vector<MeshData>md;
			for (auto& go : sceneData.sceneIDs)
			{
				//Check name tag
				ecs::NameComponent* nc = m_ecs.GetComponent<NameComponent>(go);
				//Check if game object tags match
				bool matchcon = false;
				for (std::string& tag : addedTags) {
					if (tag == nc->entityTag) {
						matchcon = true;
						break;;
					}
				}
				TransformComponent* transform = m_ecs.GetComponent<TransformComponent>(go);
				MeshFilterComponent* mfc = m_ecs.GetComponent<MeshFilterComponent>(go);
				if (matchcon && mfc) {
					//Add mesh renderer to entity
					//Get resource
					std::vector<PBRMaterial>pbrTmpList;
					pbrTmpList.push_back(PBRMaterial{});

					std::shared_ptr<R_Model> mesh = m_resourceManager.GetResource<R_Model>(mfc->meshGUID);
					//std::cout << "Mesh stuff " << mfc->meshGUID.GetToString()<<' '<<go << '\n';
					if (mesh)md.emplace_back(MeshData{ mesh,std::make_shared<PBRMaterialList>(pbrTmpList,true), transform->transformation,go });

				}
			}
			for (auto& lcComp : sceneData.sceneIDs)
			{
				if (!m_ecs.HasComponent<LightComponent>(lcComp))continue;;
				//std::cout << lcComp << "<- LIGHT ENTITY\n";
				//std::cout << "INDEX: " << i << '\n';
				//Add shadow setting later as well
				if (!m_ecs.GetComponent<ecs::LightComponent>(lcComp)->bakedLighting) {
					i++;
					continue;;
				}
				//std::cout << m_clickedEntityId << "<- SELECTED LIGHT ENTITY\n\n";

				//std::cout<<"Material list Size " << reinterpret_cast<PBRMaterialList*>(md[0].meshMaterial.get())->pbrMatList.size() << '\n';
				//EVENTUALLY MAKE IT DO ITS OWN DEPTH BUFFER CREATION
				//Get DCM, make a faux depth map renderer
				//Get all objects
				m_graphicsManager.gm_FillDepthCube(CameraData{}, 0, m_ecs.GetComponent<ecs::TransformComponent>(lcComp)->WorldTransformation.position, md);

				//Asset creation
				//Generate GUID
				// Attach name
				//KEEP THIS
				std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/DepthMap/" + std::to_string(lcComp) + ".dcm";
				m_graphicsManager.lightRenderer.dcm[0].SaveDepthCubeMap(filepath);


				//Add and load asset and assign it to light component
				//Need to change entity itself
				//std::cout << lcComp << '\n';
				m_ecs.GetComponent<ecs::LightComponent>(lcComp)->depthMapGUID.SetFromString(m_assetManager.RegisterAsset(filepath).GetToString());
				//std::cout << "Depth map GUID " << m_ecs.GetComponent<ecs::LightComponent>(lcComp)->depthMapGUID.GetToString();
				m_assetManager.Compilefile(filepath);
				i++;

				//Retrieve GUID from file path
			}
		}
		if (ImGui::Button("Test Lights")) {
			for (int i = 0; i < addedTags.size(); i++)
			{
				//std::cout << addedTags[i] << '\n';
			}
		}


		//if (ImGui::Button("Save DCM")) {
		//	auto sceneData = m_ecs.GetSceneData(m_activeScene);
		//	int i = 0;
		//	for (auto& lcComp : m_graphicsManager.lightRenderer.pointLightsToDraw) {
		//		i++;
		//		std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/DepthMap/" + std::to_string(lcComp.position.x + lcComp.position.y + lcComp.position.z) + ".dcm";

		//		if (!lcComp.bakedCon)continue;;
		//		m_graphicsManager.lightRenderer.dcm[i - 1].SaveDepthCubeMap(filepath);


		//		//Add and load asset and assign it to light component
		//		//Need to change entity itself
		//		//std::cout << lcComp << '\n';
		//		lcComp.bakedmapGUID.SetFromString(m_assetManager.RegisterAsset(filepath).GetToString());
		//		//std::cout << "Depth map GUID " << m_ecs.GetComponent<ecs::LightComponent>(lcComp)->depthMapGUID.GetToString();
		//		m_assetManager.Compilefile(filepath);
		//		//break;;
		//		//Retrieve GUID from file path
		//	}
		//	for (auto& lcComp : sceneData.sceneIDs)
		//	{
		//	}
		//}
		if (ImGui::Button("Save DCM")) {
			auto sceneData = m_ecs.GetSceneData(m_activeScene);
			int i = 0;
			for (auto& lcComp : sceneData.sceneIDs)
			{
				if (m_ecs.HasComponent<LightComponent>(lcComp) && m_ecs.GetComponent<ecs::LightComponent>(lcComp)->lightType==1&& !m_ecs.GetComponent<NameComponent>(lcComp)->hide) {
					//std::cout << m_ecs.GetComponent<ecs::LightComponent>(lcComp)->lightType << "<- LIGHT ENTITY\n";
					//std::cout << "INDEX: " << i << '\n';
					//Add shadow setting later as well
					//Asset creation
					//Generate GUID
					// Attach name
					//KEEP THIS
					//Save ALL DCMS
					//std::cout << "TEST TEST\n";
					i++;
					//Getnerate a GUID
					
					std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/DepthMap/" + utility::GenerateGUID().GetToString() + ".dcm";

					if (!m_ecs.GetComponent<ecs::LightComponent>(lcComp)->bakedLighting)continue;;

					std::cout << i - 1 << '\n';
					m_graphicsManager.lightRenderer.dcm[i - 1].SaveDepthCubeMap(filepath);


					//Add and load asset and assign it to light component
					//Need to change entity itself
					//std::cout << lcComp << '\n';
					m_ecs.GetComponent<ecs::LightComponent>(lcComp)->depthMapGUID.SetFromString(m_assetManager.RegisterAsset(filepath).GetToString());
					//std::cout << "Depth map GUID " << m_ecs.GetComponent<ecs::LightComponent>(lcComp)->depthMapGUID.GetToString();
					m_assetManager.Compilefile(filepath);
				}
				//break;;
				//Retrieve GUID from file path
			}
		}
	}

	ImGui::End();
}
