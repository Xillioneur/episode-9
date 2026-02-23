#ifndef LEVEL_MANAGER_H
#define LEVEL_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <vector>
#include <map>
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
    bool fromPlayer; 
};

typedef enum MissionType {
    BANISH_DOUBT, 
    RESTORE_LIGHT, 
    WALK_OF_FAITH, 
    BOSS_TRIAL     
} MissionType;

class LevelManager {
public:
    std::string currentLevelName;
    MissionType currentMissionType;
    int requiredScriptures;
    int scripturesFoundInLevel;

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
    std::map<std::string, Vector3> spawnPoints; 
    std::map<std::string, Vector3> exitPoints;  
};

#endif // LEVEL_MANAGER_H
