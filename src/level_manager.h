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

struct Decoration {
    Vector3 position;
    float height;
    float radius;
    int type; // 0 for tree, 1 for crystal, 2 for pillar
};

typedef enum MissionType {
    BANISH_DOUBT, 
    RESTORE_LIGHT, 
    WALK_OF_FAITH, 
    BOSS_TRIAL     
} MissionType;

typedef enum Level {
    HUB = 0,
    DAY_1, DAY_2, DAY_3, DAY_4, DAY_5,
    DAY_6, DAY_7, DAY_8, DAY_9, DAY_10,
    DAY_11, DAY_12, DAY_13, DAY_14, DAY_15
} Level;

class LevelManager {
public:
    Level currentLevel;
    std::string currentLevelName;
    MissionType currentMissionType;
    int requiredScriptures;
    int scripturesFoundInLevel;

    std::vector<Scripture> currentLevelScriptures;
    std::set<std::string> allFoundScriptureIDs; 
    
    std::vector<Projectile> projectiles;
    std::vector<Decoration> currentLevelDecorations;

    LevelManager();

    void GoToLevel(Level level);
    void UnloadLevel();
    void UpdateCurrentLevel(float dt, Player& player);
    void DrawCurrentLevel(Shader lightingShader, int lightPosLoc, int viewPosLoc, Vector3 lightPosition, Vector3 cameraPosition);

    void AddProjectile(Vector3 pos, Vector3 vel, float rad, float life, bool fromP);

    Vector3 GetSpawnPoint();
    Vector3 GetExitPosition();

private:
    std::map<Level, Vector3> spawnPoints; 
    std::map<Level, Vector3> exitPoints;  
};

#endif // LEVEL_MANAGER_H
