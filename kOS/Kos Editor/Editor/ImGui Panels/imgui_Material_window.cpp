#include "Editor.h"
#include "config/pch.h"
#include "AssetManager/AssetManager.h"
#include "Resources/ResourceManager.h"
#include "Graphics/GraphicsManager.h"

struct DisplayMaterial{
    std::string materialName;
    MaterialData data;

    REFLECTABLE(DisplayMaterial, materialName, data);
};
struct MaterialFB {
    FrameBuffer fb;
    bool startCon = false;
};
DisplayMaterial materialData;
MaterialFB mfb;
//Camera data
std::string lastGUID;
void gui::ImGuiHandler::DrawMaterialWindow() {
    if (lastGUID != selectedAsset.GUID.GetToString()&&selectedAsset.Type=="R_Material") {
        lastGUID =  selectedAsset.GUID.GetToString();
        //Update parameters
        //Getresource
        std::shared_ptr<R_Material> testMat = m_resourceManager.GetResource<R_Material>(selectedAsset.GUID);
        if (testMat) {
            //Update material list
            materialData.data.diffuseMaterialGUID = testMat->md.diffuseMaterialGUID;
            materialData.data.specularMaterialGUID = testMat->md.specularMaterialGUID;
            materialData.data.normalMaterialGUID = testMat->md.normalMaterialGUID;
            materialData.data.ambientOcclusionMaterialGUID = testMat->md.ambientOcclusionMaterialGUID;
            materialData.data.roughnessMaterialGUID = testMat->md.roughnessMaterialGUID;

        }
        else {
            //Update material list
            materialData.data.diffuseMaterialGUID = utility::GUID{};
            materialData.data.specularMaterialGUID = utility::GUID{};
            materialData.data.normalMaterialGUID = utility::GUID{};
            materialData.data.ambientOcclusionMaterialGUID = utility::GUID{};
            materialData.data.roughnessMaterialGUID = utility::GUID{};
        }
    }
    if (!mfb.startCon) {
        mfb.fb.InitializeFBO(350, 350);
        mfb.startCon = true;
    }
    ImGui::Begin("Material");
    float windowWidth = ImGui::GetContentRegionAvail().x; // Available width inside window

     
    std::shared_ptr<R_Texture> diff = m_resourceManager.GetResource<R_Texture>(materialData.data.diffuseMaterialGUID);
    std::shared_ptr<R_Texture> spec = m_resourceManager.GetResource<R_Texture>(materialData.data.specularMaterialGUID);
    std::shared_ptr<R_Texture> norm = m_resourceManager.GetResource<R_Texture>(materialData.data.normalMaterialGUID);
    std::shared_ptr<R_Texture> ao = m_resourceManager.GetResource<R_Texture>(materialData.data.ambientOcclusionMaterialGUID);
    std::shared_ptr<R_Texture> rough = m_resourceManager.GetResource<R_Texture>(materialData.data.roughnessMaterialGUID);
    PBRMaterial pbrMat{ diff,spec,rough,ao,norm };
    Camera cam;
   m_graphicsManager.gm_DrawMaterial(pbrMat, mfb.fb);

    ImGui::Image(
        reinterpret_cast<void*>(static_cast<uintptr_t>(mfb.fb.texID)),
        ImVec2(350, 350),        // Size
        ImVec2(0, 0),            // UV top-left
        ImVec2(1, 1));           // UV bottom-right
    

 
    materialData.ApplyFunction(DrawComponents{ materialData.Names() });



    float buttonWidth = 100.0f; // Width of your button

    ImGui::SetCursorPosX((windowWidth - buttonWidth) * 0.5f); // Center horizontally
    if(ImGui::Button("Create", ImVec2(buttonWidth, 0))){
        if (!materialData.materialName.empty()) {
            std::string fileName = materialData.materialName + ".mat";
            std::string filepath = m_assetManager.GetAssetManagerDirectory() + "/Material/" + fileName;

            serialization::WriteJsonFile(filepath, &materialData.data);
           // std::cout << "TEST CREATION\n";
            LOGGING_POPUP("Material Successfully Added");
        }
        else {
			LOGGING_WARN("Material Name cannot be empty");
        }


    }


    ImGui::End();

}