#include "AssetManager.h"
#include "DeSerialization/json_handler.h"
#include "Compilers/Compiler.h"
#include "Configs/ConfigPath.h"
#include "ECS/ECS.h"
namespace {
    // Drop-in replacement for std::system() that bypasses cmd.exe entirely
    int LaunchCompilerProcess(const std::string& exePath, const std::string& arguments) {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        // CreateProcess requires the full command line (EXE path + arguments)
        std::string fullCommand = "\"" + exePath + "\" " + arguments;

        // CreateProcessA requires a mutable char array
        std::vector<char> cmdBuffer(fullCommand.begin(), fullCommand.end());
        cmdBuffer.push_back('\0');

        if (!CreateProcessA(
            exePath.c_str(),        // Exact path to the .exe
            cmdBuffer.data(),       // Mutable command string
            NULL, NULL, FALSE, 0, NULL, NULL,
            &si, &pi
        )) {
            return static_cast<int>(GetLastError()); // Return actual Windows Error Code
        }

        // Wait for the compiler to finish
        WaitForSingleObject(pi.hProcess, INFINITE);

        DWORD exitCode;
        GetExitCodeProcess(pi.hProcess, &exitCode);

        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

        return static_cast<int>(exitCode);
    }
}

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
    if (m_assetWatcher) {
        m_assetWatcher->Stop();
    }

    std::lock_guard<std::mutex> lock(m_compilationMutex);
    for (auto& future : m_activeCompilations) {
        if (future.valid()) {
            future.wait();
        }
    }
}


void AssetManager::Init(const std::string& assetDirectory, const std::string& resourceDirectory)
{
    m_assetDirectory = assetDirectory;
    m_resourceDirectory = resourceDirectory;

    std::vector<std::future<void>> initTasks;

    // Determine a safe number of concurrent threads based on the CPU
    const size_t maxConcurrentTasks = std::thread::hardware_concurrency();

    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_assetDirectory))
    {
        if (entry.is_directory()) continue;
        std::filesystem::path filePath = entry.path();
        if (filePath.extension() == ".meta") continue;

        std::string inputExtension = filePath.extension().string();
        if (m_compilerMap.find(inputExtension) == m_compilerMap.end()) continue;

        RegisterAsset(filePath);

        // --- THROTTLING LOGIC ---
        // If we hit our CPU limit, wait for at least one task to finish before spawning a new one
        if (initTasks.size() >= maxConcurrentTasks) {
            bool taskFinished = false;
            while (!taskFinished) {
                for (auto it = initTasks.begin(); it != initTasks.end(); ++it) {
                    if (it->wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                        initTasks.erase(it);
                        taskFinished = true;
                        break;
                    }
                }
                if (!taskFinished) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Prevent CPU pegging
                }
            }
        }

        initTasks.push_back(std::async(std::launch::async, [this, filePath, inputExtension]() {
            std::filesystem::path metaPath = filePath;
            metaPath += ".meta";

            if (!std::filesystem::exists(metaPath)) {
                Compilefile(filePath).wait();
                return;
            }

            AssetData data = serialization::ReadJsonFile<AssetData>(metaPath.string());
            const auto& compilers = m_compilerMap.at(inputExtension);
            bool needsCompilation = false;

            for (const auto& compilerData : compilers) {
                std::filesystem::path outputResourcePath = std::filesystem::absolute(m_resourceDirectory) / (data.GUID.GetToString() + compilerData.outputExtension);
                if (!std::filesystem::exists(outputResourcePath)) {
                    needsCompilation = true;
                    break;
                }
            }

            if (needsCompilation) {
                Compilefile(filePath).wait();
            }
            }));
    }

    // Wait for the remaining tail-end tasks to finish
    for (auto& task : initTasks) {
        if (task.valid()) {
            task.get();
        }
    }

    m_assetWatcher = std::make_unique<Watcher>(m_assetDirectory, std::chrono::milliseconds(1000));

    m_assetWatcher->SetCallback([&](ACTION action, const std::filesystem::path& filePath) {

        CleanupFinishedCompilations();

        switch (action)
        {
        case ADDED: {
            LOGGING_INFO("Watcher: Added - " + filePath.string());
            RegisterAsset(filePath);

            std::lock_guard<std::mutex> lock(m_compilationMutex);
            m_activeCompilations.push_back(Compilefile(filePath));
            break;
        }
        case MODIFIED: {
            LOGGING_INFO("Watcher: Modified - " + filePath.string());

            std::lock_guard<std::mutex> lock(m_compilationMutex);
            m_activeCompilations.push_back(Compilefile(filePath));
            break;
        }
        case REMOVED:
            LOGGING_INFO("Watcher: Removed - " + filePath.string());
            break;
        default:
            break;
        }
        });

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

    std::unique_lock lock(m_mapMutex);
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
    if (m_compilerMap.find(inputExtension) == m_compilerMap.end()) {
        return std::async(std::launch::deferred, []() {});
    }

    std::filesystem::path metaPath = filePath;
    metaPath += ".meta";

    if (!std::filesystem::exists(metaPath)) {
        RegisterAsset(filePath);
        if (!std::filesystem::exists(metaPath)) {
            LOGGING_WARN("Compile File: Meta file does not exist after registering asset");
            return std::async(std::launch::deferred, []() {});
        }
    }

    AssetData data = serialization::ReadJsonFile<AssetData>(metaPath.string());
    const auto& map = m_compilerMap.at(inputExtension);

    // Store all tasks for THIS file here
    std::vector<std::future<void>> compilerTasks;

    for (const auto& compilerData : map)
    {
        std::string guid = data.GUID.GetToString();
        std::string inputPath = std::filesystem::absolute(filePath).string();
        std::string outputResourcePath = std::filesystem::absolute(m_resourceDirectory).string() + "\\" + guid + compilerData.outputExtension;

        if (compilerData.compilerFilePath != "null")
        {
            std::string compilerPath = std::filesystem::absolute(compilerData.compilerFilePath).string();
            std::string absmetaPath = std::filesystem::absolute(metaPath).string();

            // CLEAN STRING BUILDING: Just the arguments, no outer wrapper quotes needed!
            std::string arguments = "\"" + inputPath + "\" \"" + absmetaPath + "\" \"" + outputResourcePath + "\"";
			std::cout << "COMMAND DEBUG: " << arguments << std::endl; 
            compilerTasks.push_back(std::async(std::launch::async, [compilerPath, arguments]() {

                // Call our robust native Windows API helper
                int result = LaunchCompilerProcess(compilerPath, arguments);

                if (result != 0) {
                    std::cerr << "Compiler failed with code: " << result
                        << "\nEXE: " << compilerPath
                        << "\nArgs: " << arguments << std::endl;
                }
                }));
        }
        else
        {
            std::filesystem::path dir = std::filesystem::path(outputResourcePath).parent_path();
            if (!std::filesystem::exists(dir)) {
                std::filesystem::create_directories(dir);
            }
            std::filesystem::copy_file(inputPath, outputResourcePath, std::filesystem::copy_options::overwrite_existing);
        }
    }

    // Return a single future that waits for all compilers to finish on this specific file
    return std::async(std::launch::async, [tasks = std::move(compilerTasks)]() mutable {
        for (auto& t : tasks) {
            if (t.valid()) {
                t.wait();
            }
        }
        });
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

void AssetManager::CleanupFinishedCompilations() {
    std::lock_guard<std::mutex> lock(m_compilationMutex);

    // Erase-remove idiom to filter out finished futures
    m_activeCompilations.erase(
        std::remove_if(m_activeCompilations.begin(), m_activeCompilations.end(),
            [](const std::future<void>& f) {
                // Check if the future is ready without actually waiting
                return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
            }),
        m_activeCompilations.end()
    );
}

