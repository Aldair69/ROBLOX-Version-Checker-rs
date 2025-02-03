#define _CRT_SECURE_NO_WARNINGS // Disable deprecation warnings for _dupenv_s
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <windows.h>

// Structure to hold version information (date and type)
struct VersionInfo {
    std::string date;
    std::string type;
};

// Function to read the embedded deployhistory.txt resource from the executable
std::string ReadEmbeddedFile() {
    HMODULE hModule = GetModuleHandle(nullptr);
    HRSRC hResource = FindResource(hModule, MAKEINTRESOURCE(101), RT_RCDATA);
    if (!hResource) {
        std::cerr << "could not find embedded deployhistory.txt, complain to lordarathres2 about this" << std::endl;
        return "";
    }

    HGLOBAL hMemory = LoadResource(hModule, hResource);
    if (!hMemory) {
        std::cerr << "could not load embedded deployhistory.txt, complain to lordarathres2 about this" << std::endl;
        return "";
    }

    DWORD size = SizeofResource(hModule, hResource);
    const char* data = static_cast<const char*>(LockResource(hMemory));

    return std::string(data, size);
}

// Function to extract hashes and their corresponding date and type from deployhistory.txt
std::unordered_map<std::string, VersionInfo> ExtractHashes(const std::string& deployHistory) {
    std::unordered_map<std::string, VersionInfo> hashes;
    std::istringstream stream(deployHistory);
    std::string line;

    while (std::getline(stream, line)) {
        size_t firstComma = line.find(',');
        size_t secondComma = line.find(',', firstComma + 1);

        if (firstComma != std::string::npos && secondComma != std::string::npos) {
            std::string hash = line.substr(8, firstComma - 8); // Extract hash after "version-"
            std::string date = line.substr(firstComma + 1, secondComma - firstComma - 1); // Extract date
            std::string type = line.substr(secondComma + 1); // Extract type

            hashes[hash] = { date, type }; // Store the version info for the hash
        }
    }

    return hashes;
}

// Function to search the %LOCALAPPDATA%\Roblox\Versions directory for matching hashes
void SearchVersions(const std::unordered_map<std::string, VersionInfo>& hashes) {
    std::string localAppData = std::getenv("LOCALAPPDATA");
    std::string versionsDir = localAppData + "\\Roblox\\Versions";

    if (!std::filesystem::exists(versionsDir)) {
        std::cerr << "versions directory does not exist at " << versionsDir << ", you dont have roblox installed at all" << std::endl;
        return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(versionsDir)) {
        if (entry.is_directory()) {
            std::string folderName = entry.path().filename().string();
            if (folderName.find("version-") == 0) { // Check if folder starts with "version-"
                std::string hash = folderName.substr(8); // Extract hash after "version-"

                // If the hash exists in the deployhistory.txt hashes map, print its information
                if (hashes.find(hash) != hashes.end()) {
                    const auto& versionInfo = hashes.at(hash);
                    std::cout << "Found version hash: " << hash
                        << " of date: " << versionInfo.date
                        << " of type: " << versionInfo.type << std::endl;
                }
            }
        }
    }
}

int main() {
    std::cout << "lordarathres2's Version Locater " << std::endl;

    // Read the contents of the embedded deployhistory.txt
    std::string deployHistory = ReadEmbeddedFile();
    if (deployHistory.empty()) {
        return 1; // Exit if the file could not be read
    }

    // Extract hashes and corresponding version info from deployhistory.txt
    std::unordered_map<std::string, VersionInfo> hashes = ExtractHashes(deployHistory);

    // Search the Roblox Versions directory for matching hashes
    SearchVersions(hashes);

    system("pause");

    return 0;
}
