#include <filesystem>
#include <string>
#include <windows.h>
#include <iostream>


int main(int argc, char* argv[])
{
    if (argc != 3) {
        std::cerr << "Not enough argument" << std::endl;
    }

	std::string texturePath{ std::filesystem::absolute(argv[1]).string() };
	std::string metaPath{ std::filesystem::absolute(argv[2]).string() };
	std::string outputPath{ std::filesystem::absolute(argv[3]).string() };

    std::string cmd =
        "ffmpeg.exe -y -i \"" + texturePath +
        "\" -c:v mpeg1video -c:a mp2 \"" + outputPath + "\"";

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);

    CreateProcessA(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}