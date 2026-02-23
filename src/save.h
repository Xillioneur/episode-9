#ifndef SAVE_SYSTEM_H
#define SAVE_SYSTEM_H

#include "raylib.h"
#include <string>
#include <vector>
#include <map>

// Forward declarations if needed for classes used in GameData
// For now, GameData only uses Raylib types and basic C++ types.

// Struct to hold game data that needs to be saved
struct GameData {
    Vector3 playerPosition;
    float playerFaith;
    int currentDay;
    std::vector<Vector3> banishedEnemyPositions; // To track persistence
    std::vector<std::string> collectedScriptureIDs; // Track found lore
};

// SaveSystem class to handle saving and loading game data
class SaveSystem {
public:
    // Static method to save game data to a file
    static bool SaveGame(const GameData& data, const std::string& filename);

    // Static method to load game data from a file
    static GameData LoadGame(const std::string& filename);

private:
    // Helper functions for serializing/deserializing Vector3 to/from string
    static std::string Vector3ToString(const Vector3& vec);
    static Vector3 StringToVector3(const std::string& str);
};

#endif // SAVE_SYSTEM_H
