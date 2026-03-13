#include "Config/pch.h"
#include "R_PostProcessProfile.h"
#include "Graphics/ShaderManager.h"
#include "..\DeSerialization\json_handler.h"
void R_PostProcessingProfile::Load()
{
	
	//std::cout << "Loaded post processing profile"<< this->GetFilePath().string()<<'\n';
	std::ifstream inputFile(this->GetFilePath().string());

	if (!inputFile) {
		throw std::runtime_error(this->GetFilePath().string());
	}
	std::string fileContent((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	// Parse the JSON content
	rapidjson::Document doc;
	doc.Parse(fileContent.c_str());
	if (doc.HasMember("ProfileName")) {
		this->profile.profileName = doc["ProfileName"].GetString();
	}
	const rapidjson::Value& effects = doc["PostProfileEffects"];

	for (rapidjson::SizeType i = 0; i < effects.Size(); ++i) {
		const rapidjson::Value& e = effects[i];

		int ppe = e["Type"].GetInt();
		switch (ppe) {
			//Vigniette
			case 0:
			{
				//Get float and get 
				Vigniette vig;
				vig.intensity = e["Intensity"].GetFloat();
				vig.extent = e["Extent"].GetFloat();
				if (e.HasMember("Color")) {
					const auto& colorArr = e["Color"].GetArray();
					vig.color = glm::vec3(
						colorArr[0].GetFloat(),
						colorArr[1].GetFloat(),
						colorArr[2].GetFloat()
					);
				}
	
				//Get color array
				// 
				//Push data in 
				//vig.currentShader = ;
				this->profile.postProcessingEffects.push_back(std::make_unique<Vigniette>(vig));
				break;;
			}
			case 1:
			{
				FilmGrain fg;
				fg.noiseStrength = e["NoiseStrength"].GetFloat();
				this->profile.postProcessingEffects.push_back(std::make_unique<FilmGrain>(fg));
				break;;
			}
			case 2:
			{
				ChromaticAberration ca;
				ca.redOffset = e["RedOffset"].GetFloat();
				ca.greenOffset = e["GreenOffset"].GetFloat();
				ca.blueOffset = e["BlueOffset"].GetFloat();
				this->profile.postProcessingEffects.push_back(std::make_unique<ChromaticAberration>(ca));

				break;;
			}
			case 3:
			{
				Blur blur;
				blur.radius = e["Radius"].GetFloat();
				this->profile.postProcessingEffects.push_back(std::make_unique<Blur>(blur));
				break;;
			}
			case 4:
				Bloom blur;
				blur.bloomStrength = e["BloomStrength"].GetFloat();
				this->profile.postProcessingEffects.push_back(std::make_unique<Bloom>(blur));
				break;;

				break;;


		}
	}
	//this->md = serialization::ReadJsonFile<MaterialData>(this->GetFilePath().string());
	////std::cout << "MATERIAL PATH" << this->GetFilePath().string() << '\n';
	////std::cout << "MATERIAL LOADED" << this->md.diffuseMaterialGUID.GetToString() << '\n';
	////Get resources from texture guid and load them

}
void R_PostProcessingProfile::Unload() {}

