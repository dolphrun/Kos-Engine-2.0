#pragma once

#include "Config/pch.h"
#include "AssetDatabase.h"
#include "Watcher.h"
#include "Resources/R_Texture.h"

class AssetManager {

public:

    AssetManager();

    ~AssetManager();

    void Init(const std::string& assetDirectory, const std::string& resourceDirectory);

    utility::GUID RegisterAsset(const std::filesystem::path& filepath);

    std::future<void> Compilefile(const std::filesystem::path& filepath);

    inline std::string GetTypefromExtension(std::string extension) {
        if (m_extensionRegistry.find(extension) == m_extensionRegistry.end()) {
            throw std::runtime_error("Unknown extension: " + extension);
        }


        return m_extensionRegistry.at(extension);
    }

    inline utility::GUID GetGUIDfromFilePath(std::filesystem::path filepath) {

        utility::GUID GUID;
        try {
            GUID = m_dataBase.GetGUID(filepath);
        }
        catch (const std::runtime_error& e) {
            LOGGING_WARN("Runtime error: " + std::string(e.what()));
            return  utility::GUID();
        }

        return GUID;
    }

	std::string GetAssetManagerDirectory() const { return m_assetDirectory; }


    Watcher* GetAssetWatcher() {
        return m_assetWatcher.get();
    }

private:

    //template<typename T, typename... U>
    //void RegisterAssetType(const std::string& compilerFilePath, const std::string& outputExtension, U&&... type) {
    //    std::string className = T::classname();

    //    //m_CompilerPath[className] = { compilerFilePath,outputExtension };

    //    //fold
    //    ((m_extensionRegistry[type] = T::classname()), ...);
    //}


private:

    //Key - GUID
    std::unordered_map <utility::GUID, std::filesystem::path> m_GUIDtoFilePath;

    //extension registry
    std::unordered_map<std::string, std::string> m_extensionRegistry;

    //Asset Database
    AssetDatabase m_dataBase;

	//Asset Directory
	std::string m_assetDirectory;
    std::string m_resourceDirectory;

    //Watcher
    std::unique_ptr<Watcher> m_assetWatcher;
	//std::unique_ptr<Watcher> m_scriptWatcher;

    //Compiler Data
    struct CompilerD{
        std::string type;
        std::string compilerFilePath;
        std::string outputExtension;
    };
    std::unordered_map<std::string, std::vector<CompilerD>> m_compilerMap;
public:
    const std::unordered_map<std::string, std::vector<CompilerD>>& GetCompilerMap() const { return m_compilerMap; }
    std::unique_ptr<R_Texture> folderTexture;
    std::unique_ptr<R_Texture> fileTexture;
};

