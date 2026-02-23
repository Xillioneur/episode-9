#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <vector>
#include <map> // For storing spawn points

// Forward declarations for shader related structs if needed, but in this case,
// Shader is a raylib struct, so directly including "raylib.h" is fine.

#include <set>

class Player; // Forward declaration

struct Scripture {
    std::string id;
    std::string text;
    Vector3 position;
    bool found;
};

struct Projectile {
    Vector3 position;
    Vector3 velocity;
    float radius;
    float lifetime;
    bool active;
    bool fromPlayer; // To distinguish between player and enemy projectiles
};

class LevelManager {
public:
    std::string currentLevelName;
    std::vector<Scripture> currentLevelScriptures;
    std::set<std::string> allFoundScriptureIDs; // Persists across level loads
    
    std::vector<Projectile> projectiles;

    LevelManager();

    void LoadLevel(const std::string& levelName);
    void UnloadLevel();
    void UpdateCurrentLevel(float dt, Player& player);
    void DrawCurrentLevel(Shader lightingShader, int lightPosLoc, int viewPosLoc, Vector3 lightPosition, Vector3 cameraPosition);

    void AddProjectile(Vector3 pos, Vector3 vel, float rad, float life, bool fromP);

    Vector3 GetSpawnPoint(const std::string& levelName);
    Vector3 GetExitPosition(const std::string& levelName);

private:
    std::map<std::string, Vector3> spawnPoints; // Store spawn points for levels
    std::map<std::string, Vector3> exitPoints;  // Store exit/altar points for levels
};

#endif // LEVEL_MANAGER_H
