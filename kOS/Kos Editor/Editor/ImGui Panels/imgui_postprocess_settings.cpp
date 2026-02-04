#include "Editor.h"
#include "config/pch.h"
#include "AssetManager/AssetManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/GraphicsManager.h"

namespace postProcessSettings{
	//PostProcessingProfile tempProfile;
	std::string lastPPGUID;
	char buffer[256];
	PostProcessingProfile* ppp{ nullptr};
	std::vector<const char*>profileNames = { "Add profile effect","Vigniette","Noise strength","Chromatic Abberation"};
}


void gui::ImGuiHandler::DrawPostProcessWindow() {
	//Display info first by double clicking click clicks
	if (postProcessSettings::lastPPGUID != selectedAsset.GUID.GetToString() && selectedAsset.Type == "R_PostProcessingProfile") {
		
		postProcessSettings::ppp = &m_resourceManager.GetResource<R_PostProcessingProfile>(selectedAsset.GUID)->profile;
		//postProcessSettings::tempProfile.profileName= ppp->profileName;
		strncpy(postProcessSettings::buffer, postProcessSettings::ppp->profileName.c_str(), 256);
		//postProcessSettings::lastPPGUID = selectedAsset.GUID.GetToString();
		////Copy unique ptr details over
		//for (auto& ptr : postProcessSettings::tempProfile.postProcessingEffects) {
		//	switch (ptr->GetType()) {
		//	case 0:
		//		Vigniette * v = reinterpret_cast<Vigniette*>(ptr.get());
		//		postProcessSettings::tempProfile.postProcessingEffects.push_back(std::make_unique<Vigniette>(*v));
		//		break;;
		//	}
		//}

;	}
	if (!postProcessSettings::ppp) {
		return;;
	}

	//Profile em rip sean
	ImGui::Begin("Post processing");
	//Set up the text buffer to input the name 

	ImGui::Text(postProcessSettings::ppp->profileName.c_str());
	int PPETypeIndex = 0;
	if (ImGui::Combo("##ADDPPE", &PPETypeIndex, postProcessSettings::profileNames.data(), static_cast<int>(postProcessSettings::profileNames.size()))) {
		switch (PPETypeIndex) {
			case 1:
				postProcessSettings::ppp->postProcessingEffects.push_back(std::make_unique<Vigniette>(Vigniette{}));
				break;;
			case 2:
				postProcessSettings::ppp->postProcessingEffects.push_back(std::make_unique<FilmGrain>(FilmGrain{}));
				break;;
			case 3:
				postProcessSettings::ppp->postProcessingEffects.push_back(std::make_unique<ChromaticAberration>(ChromaticAberration{}));
				break;;
		}
	}
	for (auto& eff : postProcessSettings::ppp->postProcessingEffects) {
		switch (eff->GetType()) {
		case PPT_Vigniette:
		{
			//Print out effect and intensity
			Vigniette* v = reinterpret_cast<Vigniette*>(eff.get());
			ImGui::Text("Vigniette");
			ImGui::DragFloat("Intensity", &v->intensity, 0.01f);
			ImGui::DragFloat("Extent", &v->extent, 0.01f);
			break;;
		}
		case PPT_FilmGrain:
		{
			//Print out effect and intensity
			FilmGrain* v = reinterpret_cast<FilmGrain*>(eff.get());
			ImGui::Text("Film Grain");
			ImGui::DragFloat("Noise Strength", &v->noiseStrength, 0.01f);
			break;;
		}
		case PPT_ChromaticAbberation:
		{
			//Print out effect and intensity
			ChromaticAberration* v = reinterpret_cast<ChromaticAberration*>(eff.get());
			ImGui::Text("Chromatic abberation");
			ImGui::DragFloat("Red Offset", &v->redOffset, 0.01f);
			ImGui::DragFloat("Green Offset", &v->greenOffset, 0.01f);
			ImGui::DragFloat("Blue Offset", &v->blueOffset, 0.01f);
			break;;
		}
		}
	}
	//Save profile 
	if (ImGui::Button("Save Profile")) {
		std::string fileName = postProcessSettings::ppp->profileName + ".prof";
		std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/PostProcessProfile/" + fileName;
		//Serialize data
		serialization::JsonFileValidation(filepath);

		rapidjson::Document doc;
		doc.SetObject(); // Initializes the document as { }
		rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

		//Add pp effects
		doc.AddMember(
			rapidjson::Value("ProfileName", allocator),
			rapidjson::Value(postProcessSettings::ppp->profileName.c_str(), allocator),
			allocator
		);
		rapidjson::Value postProfileEffects(rapidjson::kArrayType);
		for (auto& ptr : postProcessSettings::ppp->postProcessingEffects) {
			rapidjson::Value entityData(rapidjson::kObjectType);
			entityData.AddMember("Type", (int)ptr->GetType(), allocator);
			switch (ptr->GetType()) {
				case 0:
				{
					Vigniette* v = reinterpret_cast<Vigniette*>(ptr.get());
					entityData.AddMember("Intensity", v->intensity, allocator);
					entityData.AddMember("Extent", v->extent, allocator);
					break;;
				}
				case 1:
				{
					FilmGrain* fg = reinterpret_cast<FilmGrain*>(ptr.get());
					entityData.AddMember("NoiseStrength", fg->noiseStrength, allocator);
					break;;
				}
				case 2:
				{
					ChromaticAberration* ca = reinterpret_cast<ChromaticAberration*>(ptr.get());
					entityData.AddMember("RedOffset", ca->redOffset, allocator);
					entityData.AddMember("GreenOffset", ca->greenOffset, allocator);
					entityData.AddMember("BlueOffset", ca->blueOffset, allocator);
					break;;
				}

			}
			postProfileEffects.PushBack(entityData, allocator);
		}

		doc.AddMember("PostProfileEffects", postProfileEffects, allocator);
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		doc.Accept(writer);

		// 6. Override the file
		std::ofstream ofs(filepath, std::ios::out | std::ios::trunc);
		if (ofs.is_open()) {
			ofs << buffer.GetString();
			ofs.close();
		}
	}

	
	//Save profile name to profile

	ImGui::End();

}