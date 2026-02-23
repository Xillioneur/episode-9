#include "save.h"
#include "raylib.h"
#include <fstream>
#include <sstream>
#include <iomanip> // For std::fixed and std::setprecision

// Helper: Convert Vector3 to string "(x,y,z)"
std::string SaveSystem::Vector3ToString(const Vector3& vec) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(3) << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
    return ss.str();
}

// Helper: Parse string "(x,y,z)" to Vector3
Vector3 SaveSystem::StringToVector3(const std::string& str) {
    Vector3 vec = {0.0f, 0.0f, 0.0f};
    if (str.length() < 7 || str.front() != '(' || str.back() != ')') return vec;

    std::string cleanStr = str.substr(1, str.length() - 2); // Remove parentheses
    std::stringstream ss(cleanStr);
    std::string segment;

    std::getline(ss, segment, ',');
    vec.x = std::stof(segment);
    std::getline(ss, segment, ',');
    vec.y = std::stof(segment);
    std::getline(ss, segment, ',');
    vec.z = std::stof(segment);

    return vec;
}

// Save game data to a file
bool SaveSystem::SaveGame(const GameData& data, const std::string& filename) {
    std::stringstream ss;
    ss << "playerPosition=" << Vector3ToString(data.playerPosition) << "\n";
    ss << "playerFaith=" << std::fixed << std::setprecision(3) << data.playerFaith << "\n";
    ss << "currentDay=" << data.currentDay << "\n";
    
    ss << "banishedEnemyPositions=";
    for (const auto& pos : data.banishedEnemyPositions) {
        ss << Vector3ToString(pos) << ";"; 
    }
    ss << "\n";

    ss << "collectedScriptureIDs=";
    for (const auto& id : data.collectedScriptureIDs) {
        ss << id << ";";
    }
    ss << "\n";

    std::string dataString = ss.str();
    bool success = SaveFileText(filename.c_str(), (char*)dataString.c_str());

    if (success) {
        TraceLog(LOG_INFO, "SAVE: Game saved successfully to %s", filename.c_str());
    } else {
        TraceLog(LOG_ERROR, "SAVE: Failed to save game to %s", filename.c_str());
    }
    return success;
}

// Load game data from a file
GameData SaveSystem::LoadGame(const std::string& filename) {
    GameData data = {}; // Initialize with default values
    data.playerPosition = {0.0f, 0.5f, 0.0f}; // Default
    data.playerFaith = 100.0f; // Default
    data.currentDay = 1; // Default

    char* fileContent = LoadFileText(filename.c_str());
    if (fileContent == nullptr) {
        TraceLog(LOG_WARNING, "SAVE: Save file %s not found or could not be loaded. Returning default GameData.", filename.c_str());
        return data;
    }

    std::stringstream ss(fileContent);
    std::string line;

    while (std::getline(ss, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        if (key == "playerPosition") {
            data.playerPosition = StringToVector3(value);
        } else if (key == "playerFaith") {
            data.playerFaith = std::stof(value);
        } else if (key == "currentDay") {
            data.currentDay = std::stoi(value);
        } else if (key == "banishedEnemyPositions") {
            std::stringstream vs(value);
            std::string segment;
            data.banishedEnemyPositions.clear();
            while (std::getline(vs, segment, ';')) {
                if (!segment.empty()) {
                    data.banishedEnemyPositions.push_back(StringToVector3(segment));
                }
            }
        } else if (key == "collectedScriptureIDs") {
            std::stringstream vs(value);
            std::string segment;
            data.collectedScriptureIDs.clear();
            while (std::getline(vs, segment, ';')) {
                if (!segment.empty()) {
                    data.collectedScriptureIDs.push_back(segment);
                }
            }
        }
        // Add more parsing for other game state variables
    }

    UnloadFileText(fileContent); // Free the memory allocated by LoadFileText
    TraceLog(LOG_INFO, "SAVE: Game loaded successfully from %s", filename.c_str());
    return data;
}
