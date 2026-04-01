#include <Windows.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdint> // Added for uint32_t

#include <RAPIDJSON/rapidjson.h>
#include <RAPIDJSON/document.h>
#include <RAPIDJSON/error/en.h>

std::string getShortPath(const std::string& longPath) {
	char shortPath[MAX_PATH];
	DWORD result = GetShortPathNameA(longPath.c_str(), shortPath, MAX_PATH);
	if (result > 0 && result < MAX_PATH) {
		return std::string(shortPath);
	}
	return longPath; // fallback
}

// Fast helper to read width/height directly from a PNG header
bool GetPNGDimensions(const std::string& filepath, uint32_t& outWidth, uint32_t& outHeight) {
	std::ifstream file(filepath, std::ios::binary);
	if (!file) return false;

	unsigned char header[24];
	if (!file.read(reinterpret_cast<char*>(header), 24)) return false;

	// Verify it's actually a PNG (Magic Number check)
	if (header[0] != 0x89 || header[1] != 0x50 || header[2] != 0x4E || header[3] != 0x47) {
		return false;
	}

	// Extract width and height from the IHDR chunk (Big Endian)
	outWidth = (header[16] << 24) | (header[17] << 16) | (header[18] << 8) | header[19];
	outHeight = (header[20] << 24) | (header[21] << 16) | (header[22] << 8) | header[23];
	return true;
}

int main(int argc, char* argv[])
{
	std::string texturePath{ std::filesystem::absolute(argv[1]).string() };
	std::string metaPath{ std::filesystem::absolute(argv[2]).string() };
	std::string outputPath{ std::filesystem::absolute(argv[3]).string() };

	// Manipulate output file path (Remove file name)
	auto endStringIterator{ outputPath.end() };
	endStringIterator--;
	while (*endStringIterator != '\\') {
		endStringIterator--;
	}
	std::string outputFilePath = std::string(outputPath.begin(), endStringIterator);

	// Get file name
	std::filesystem::path p(texturePath);
	std::string fileName = p.stem().string();

	std::cout << "Output file path: " << outputFilePath << '\n';
	std::cout << "Texture file path: " << texturePath << '\n';

	// Open texture META file path settings
	std::ifstream ifs(metaPath);
	if (!ifs.is_open()) {
		std::cerr << "Could not open file: " << metaPath << std::endl;
		return 1;
	}
	std::stringstream buffer;
	buffer << ifs.rdbuf();
	std::string jsonStr = buffer.str();

	rapidjson::Document doc;
	doc.Parse(jsonStr.c_str());

	bool hf, vf;
	double width, height;
	int mipLevel, wrapSetting, colorSpace;

	const auto& arrObj = doc[1];
	const auto& assetData = arrObj["TextureCompilerData"];
	hf = assetData["HoriztonalFlip"].GetBool();
	vf = assetData["VerticalFlip"].GetBool();
	width = assetData["Width"].GetDouble();
	height = assetData["Height"].GetDouble();
	mipLevel = assetData["MipLevels"].GetInt();
	wrapSetting = assetData["Wrap"].GetInt();
	colorSpace = assetData["ColorSpace"].GetInt();

	std::cout << "Horizontal Flip: " << hf << std::endl;
	std::cout << "Vertical Flip: " << vf << std::endl;
	std::cout << "Width (JSON): " << width << std::endl;
	std::cout << "Height (JSON): " << height << std::endl;
	std::cout << "mipLevel: " << mipLevel << std::endl;
	std::cout << "wrapSetting: " << wrapSetting << std::endl;
	std::cout << "colorSpace: " << colorSpace << std::endl;

	std::string srgbSetting;
	switch (colorSpace) {
	case 0:
		srgbSetting = "-srgb ";
		break;
	case 1:
		srgbSetting = "-srgbi ";
		break;
	case 2:
		srgbSetting = "-srgbo ";
		break;
	}

	// --- DIMENSION CORRECTION LOGIC ---
	uint32_t actualWidth = static_cast<uint32_t>(width);
	uint32_t actualHeight = static_cast<uint32_t>(height);

	// If the JSON said width/height is 0 (keep original), read the file directly
	if (actualWidth == 0 || actualHeight == 0) {
		if (!GetPNGDimensions(texturePath, actualWidth, actualHeight)) {
			std::cout << "WARNING: Failed to read PNG dimensions for: " << texturePath << "\n";
		}
		else {
			std::cout << "SUCCESS: Read original dimensions " << actualWidth << "x" << actualHeight << " from file.\n";
		}
	}

	uint32_t safeWidth = actualWidth;
	uint32_t safeHeight = actualHeight;

	// Apply multiple-of-4 rounding fix for BC3 compatibility
	if (actualWidth > 0 && actualHeight > 0) {
		safeWidth = (actualWidth + 3) & ~3;
		safeHeight = (actualHeight + 3) & ~3;

		if (safeWidth != actualWidth || safeHeight != actualHeight) {
			std::cout << "RESIZING: " << actualWidth << "x" << actualHeight
				<< " -> " << safeWidth << "x" << safeHeight << " for BC3 compatibility.\n";
		}
	}
	// ----------------------------------

	// Build the texconv flags
	std::stringstream flags{};
	flags << "\" -ft DDS -f BC3_UNORM ";

	if (hf) flags << "-hflip ";
	if (vf) flags << "-vflip ";

	// Pass the corrected dimensions to texconv
	if (safeWidth > 0) flags << "-w " << safeWidth << " ";
	if (safeHeight > 0) flags << "-h " << safeHeight << " ";

	// Apply Mip levels and color space
	flags << "-m " << mipLevel << " ";
	flags << srgbSetting << " ";

	if (wrapSetting) {
		flags << "-mirror ";
	}

	// End flag
	flags << "-y -o \"";
	std::cout << "FLAGS: " << flags.str() << '\n';

	// Run the texconv process
	std::string exePath = std::filesystem::absolute(".\\Kos Editor\\Compilers\\Executable\\Texture Compiler\\texconv.exe").string();
	std::string command = "\"\"" + exePath + flags.str() + outputFilePath + "\" \"" + texturePath + "\"\"";

	std::cout << "COMMAND: " << command << '\n';

	int result = std::system(command.c_str());
	if (result != 0) {
		std::cout << "Command failed with code: " << result << std::endl;
	}

	// Construct the expected output path
	outputFilePath += '\\';
	outputFilePath += fileName;
	outputFilePath += ".dds";
	std::cout << "Expected output file path: " << outputFilePath << '\n';
	std::cout << "New file path: " << outputPath << '\n';

	// 1. Check if texconv actually created the file
	if (!std::filesystem::exists(outputFilePath)) {
		std::cout << "ERROR: Texture compiler failed to generate " << fileName << ".dds! (Check image dimensions or format)" << std::endl;
		return -1; // Exit gracefully instead of crashing
	}

	// 2. Use std::error_code to prevent unhandled exception crashes
	std::error_code ec;
	std::filesystem::rename(outputFilePath, outputPath, ec);

	if (ec) {
		std::cout << "ERROR: Failed to rename file: " << ec.message() << std::endl;
		return -1;
	}

	return 0;
}