#include "AssetManager.h"
#include "DeSerialization/json_handler.h"
#include "Compilers/Compiler.h"
#include "Configs/ConfigPath.h"
#include "ECS/ECS.h"



AssetManager::AssetManager()
{


    CompilerData Data = serialization::ReadJsonFile<CompilerData>(configpath::configFilePath);

    Data.ApplyFunction([&](auto& member) {
        for (const auto& inputExtension : member.inputExtensions) {
            m_compilerMap[inputExtension].emplace_back(CompilerD{ member.type, member.path, member.outputExtension });
           // std::cout << "MEMBER Input EXTENSION IS " << static_cast<std::string>(inputExtension) << '\n';
        }
        });

}

AssetManager::~AssetManager() {

}


void AssetManager::Init(const std::string& assetDirectory, const std::string& resourceDirectory)
{
	m_assetDirectory = assetDirectory;
    m_resourceDirectory = resourceDirectory;

    std::function<void(const std::string&)> readDirectory;
    std::vector<std::future<void>> compilingAsync;

    readDirectory = [&](const std::string& Dir) {
        int count = 0;
        for (const auto& entry : std::filesystem::directory_iterator(Dir)) {
            std::string filepath = entry.path().string();

            if (entry.is_directory()) {
                readDirectory(filepath); 
            }
            else {
                RegisterAsset(entry.path());

                //test if resource is already in the resource folder
                std::filesystem::path metaPath = filepath + ".meta";


                if (std::filesystem::exists(metaPath)) {
                    AssetData data = serialization::ReadJsonFile<AssetData>(metaPath.string());
                    const auto& map = m_compilerMap.at(entry.path().filename().extension().string());
                    for (const auto& compilerData : map)
                    {
                        std::string outputResourcePath = std::filesystem::absolute(m_resourceDirectory).string() + "/" + data.GUID.GetToString() + compilerData.outputExtension;
                        if (!std::filesystem::exists(outputResourcePath)) {
                            std::cout << ++count << std::endl;
                            compilingAsync.push_back(Compilefile(filepath));
                        }

                    }
                }
                else {
                    continue;
                }


            }
        }
        };


	readDirectory(m_assetDirectory);

    for (size_t i = 0; i < compilingAsync.size(); ) {
        if (compilingAsync[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            compilingAsync[i].get();
            compilingAsync.erase(compilingAsync.begin() + i);
        }
        else {
            ++i;
        }
    }


    //Setup Watchers

    m_assetWatcher = std::make_unique<Watcher>(m_assetDirectory, std::chrono::milliseconds(1000));

    m_assetWatcher->SetCallback([&](ACTION action, const std::filesystem::path& filePath) {

        switch (action)
        {
		case ADDED:
            LOGGING_INFO("Watcher: Added - " + filePath.string());
            RegisterAsset(filePath);
            Compilefile(filePath);

			break;
		case MODIFIED:
            LOGGING_INFO("Watcher: Modified - " + filePath.string());
            Compilefile(filePath);

			break;
		case REMOVED:
            LOGGING_INFO("Watcher: Removed - " + filePath.string());
			break;
		default:
			break;
        }

       
        });

    //Ignore all .meta files
    m_assetWatcher->SetIgnoreExtension(".meta");

    m_assetWatcher->Start();


    folderTexture = std::make_unique<R_Texture>(configpath::folderIconPath);
    fileTexture = std::make_unique<R_Texture>(configpath::fileIconPath);
    folderTexture->Load();
    fileTexture->Load();
}



utility::GUID AssetManager::RegisterAsset(const std::filesystem::path& filePath)
{
	std::string inputExtension = filePath.extension().string();
	//check if compiler have been registered 
	if (m_compilerMap.find(inputExtension) == m_compilerMap.end()) {
		//LOGGING_WARN("RegisterAsset: " + inputExtension + " not found");
        return utility::GUID{};
	}
    
    const std::string& type = m_compilerMap.at(inputExtension).front().type;

    utility::GUID GUID = m_dataBase.ImportAsset(filePath, type);

    m_GUIDtoFilePath[GUID] = filePath;
    return GUID;
}

std::future<void> AssetManager::Compilefile(const std::filesystem::path& filePath)
{
    if (!std::filesystem::exists(filePath)) {
        LOGGING_WARN("Compile File: " + filePath.string() + " filepath does not exist");
        return std::async(std::launch::deferred, []() {});
    }

    std::string inputExtension = filePath.extension().string();
	//check if compiler have been registered
	if (m_compilerMap.find(inputExtension) == m_compilerMap.end()) {
		//LOGGING_WARN("Compile File: " + inputExtension + "not found");
		return std::async(std::launch::deferred, []() {});
		
        //just copy the file over to the resource

	}
    
	//check if file meta exist
	std::filesystem::path metaPath = filePath.string() + ".meta";

	if (!std::filesystem::exists(metaPath)) {
        //Import asset and create meta file
        RegisterAsset(filePath);

        if (!std::filesystem::exists(metaPath)) {
			LOGGING_WARN("Compile File: Meta file does not exist after registering asset");
			return std::async(std::launch::deferred, []() {});
        }
	}
    //get file type from meta
    AssetData data = serialization::ReadJsonFile<AssetData>(metaPath.string());

    const auto& map = m_compilerMap.at(inputExtension);
    for(const auto& compilerData : map)
    {
        std::string guid = data.GUID.GetToString();

        std::string inputPath = std::filesystem::absolute(filePath).string();
        std::string outputResourcePath =
            std::filesystem::absolute(m_resourceDirectory).string() + "\\" +
            guid + compilerData.outputExtension;


        //assets that have a compiler
        if (compilerData.compilerFilePath != "null")
        {
            //COMPILING CODE HERE
            std::string compilerPath = std::filesystem::absolute(compilerData.compilerFilePath).string();
            std::string absmetaPath = std::filesystem::absolute(metaPath).string();

            // Wrap the ENTIRE command in an extra set of quotes to prevent cmd.exe from stripping them
            std::string command = "\"\"" + compilerPath + "\" \"" + inputPath + "\" \"" + absmetaPath + "\" \"" + outputResourcePath + "\"\"";

            // Run the command directly
            auto runSystemAsync = [](const std::string& cmd)
                {
                    auto future = std::async(std::launch::async, [cmd]() {
                        int result = std::system(cmd.c_str());
                        if (result != 0) {
                            std::cerr << "Command failed with code: " << result << "\nCommand: " << cmd << std::endl;
                        }
                        });

                    return future;
                };

            return runSystemAsync(command);
        }
        else {
            //assets without

            //check if directory exist, if not create one

            std::filesystem::path dir = std::filesystem::path(outputResourcePath).parent_path();

            if (!std::filesystem::exists(dir)) {
                if (!std::filesystem::create_directories(dir)) {
                    LOGGING_ERROR("Fail to create directory");
                }
            }


            std::filesystem::copy_file(inputPath, outputResourcePath, std::filesystem::copy_options::overwrite_existing);

        }

        return std::async(std::launch::deferred, []() {});
	}
    return std::async(std::launch::deferred, []() {});
}

void AssetManager::RenameFile(const std::filesystem::path& oldFile, const std::filesystem::path& newFile) {

    std::error_code ec;

    std::filesystem::rename(oldFile, newFile, ec);
    if (ec)
    {
        LOGGING_ERROR("Failed to rename asset");
        return;
    }


    std::filesystem::path oldMeta = oldFile;
    oldMeta += ".meta";

    std::filesystem::path newMeta = newFile;
    newMeta += ".meta";

    if (std::filesystem::exists(oldMeta))
    {
        std::filesystem::rename(oldMeta, newMeta, ec);
        if (ec)
        {
            LOGGING_ERROR("Failed to rename file's .meta file, a new guid will be created");
        }
    }

    LOGGING_INFO("Rename Successful");
}


