#include "level_manager.h"
#include "raylib.h"
#include "raymath.h" 
#include "player.h"

LevelManager::LevelManager() {
    currentLevelName = "";
    currentMissionType = BANISH_DOUBT;
    requiredScriptures = 0;
    scripturesFoundInLevel = 0;

    spawnPoints["Hub"] = (Vector3){0.0f, 0.5f, 0.0f}; 
    exitPoints["Hub"] = (Vector3){0.0f, 0.0f, -15.0f}; 

    for (int i = 1; i <= 15; i++) {
        std::string dayName = "Day" + std::to_string(i);
        spawnPoints[dayName] = (Vector3){0.0f, 0.5f, 0.0f};
        // Spread exit points more for variety
        float angle = (float)i * 0.8f;
        float dist = 40.0f + (i * 4.0f);
        exitPoints[dayName] = (Vector3){sinf(angle) * 20.0f, 0.0f, dist}; 
    }
}

void LevelManager::LoadLevel(const std::string& levelName) {
    currentLevelName = levelName;
    currentLevelScriptures.clear();
    projectiles.clear();
    scripturesFoundInLevel = 0;

    if (levelName == "Hub") {
        currentMissionType = WALK_OF_FAITH; 
        currentLevelScriptures.push_back({"hub_1", "John 1:5 - The light shines in the darkness.", {15.0f, 1.0f, -10.0f}, false});
    } else {
        int day = 1;
        if (levelName.length() > 3) day = std::stoi(levelName.substr(3));
        
        // Mission types cycle: Banish -> Restore -> Walk -> Banish -> Boss
        if (day == 5 || day == 10 || day == 15) currentMissionType = BOSS_TRIAL;
        else if (day % 3 == 1) currentMissionType = BANISH_DOUBT;
        else if (day % 3 == 2) currentMissionType = RESTORE_LIGHT;
        else currentMissionType = WALK_OF_FAITH;

        // scriptures
        std::string text = "Scripture for Day " + std::to_string(day);
        if (day == 1) text = "Psalm 27:1 - The LORD is my light.";
        else if (day == 2) text = "Matthew 5:14 - Ye are the light of the world.";
        else if (day == 3) text = "Ephesians 5:8 - Walk as children of light.";
        else if (day == 15) text = "Easter: He is Risen! Glory to God!";

        if (currentMissionType == RESTORE_LIGHT) {
            requiredScriptures = 3;
            for(int i=0; i<3; i++) {
                float angle = (float)i * (360.0f/3.0f) * DEG2RAD;
                currentLevelScriptures.push_back({"d" + std::to_string(day) + "_" + std::to_string(i), text, {cosf(angle)*15.0f, 1.0f, sinf(angle)*15.0f + 25.0f}, false});
            }
        } else {
            requiredScriptures = 1;
            currentLevelScriptures.push_back({"d" + std::to_string(day) + "_m", text, {0.0f, 1.0f, 20.0f}, false});
        }
    }

    for (auto& s : currentLevelScriptures) {
        if (allFoundScriptureIDs.count(s.id)) s.found = true;
    }
}

void LevelManager::UnloadLevel() {
    currentLevelName = "";
    currentLevelScriptures.clear();
    projectiles.clear();
}

void LevelManager::UpdateCurrentLevel(float dt, Player& player) {
    for (auto& p : projectiles) {
        if (!p.active) continue;
        p.lifetime -= dt;
        if (p.lifetime <= 0) { p.active = false; continue; }
        p.position = Vector3Add(p.position, Vector3Scale(p.velocity, dt));
        if (!p.fromPlayer) {
            float dist = Vector3Distance(p.position, player.position);
            if (player.isShieldActive && dist < (p.radius + 1.2f)) { p.active = false; continue; }
            if (dist < (p.radius + player.radius)) { player.TakeDamage(10.0f); p.active = false; }
        }
    }
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& p){ return !p.active; }), projectiles.end());
}

void LevelManager::DrawCurrentLevel(Shader lightingShader, int lightPosLoc, int viewPosLoc, Vector3 lightPosition, Vector3 cameraPosition) {
    BeginShaderMode(lightingShader);
        SetShaderValue(lightingShader, lightPosLoc, (float*)&lightPosition, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightingShader, viewPosLoc, (float*)&cameraPosition, SHADER_UNIFORM_VEC3);

        if (currentLevelName == "Hub") {
            DrawCube((Vector3){0.0f, -0.5f, 0.0f}, 200.0f, 1.0f, 200.0f, DARKGREEN); 
            for(int i=0; i<20; i++) {
                float angle = i * 18.0f * DEG2RAD;
                float dist = 50.0f;
                DrawCylinder({cosf(angle)*dist, 0, sinf(angle)*dist}, 0.5f, 1.0f, 5.0f, 6, BROWN);
                DrawSphere({cosf(angle)*dist, 5.5f, sinf(angle)*dist}, 3.0f, GREEN);
            }
            DrawCylinder(exitPoints["Hub"], 2.0f, 2.0f, 4.0f, 16, PURPLE); 
        }
        else {
            int day = std::stoi(currentLevelName.substr(3));
            float levelSize = 100.0f + (day * 5.0f);
            
            // UNIQUE COLOR FOR EVERY DAY
            Color groundColor = ColorFromHSV((float)((day * 24) % 360), 0.6f, 0.4f);
            DrawCube({0, -0.5f, levelSize/2}, levelSize, 1.0f, levelSize, groundColor); 

            // UNIQUE OBSTACLES PER DAY
            for (int i = 0; i < 5 + day; i++) {
                float x = sinf(i * 1.5f + day) * (levelSize * 0.4f);
                float z = cosf(i * 2.0f + day) * (levelSize * 0.4f) + levelSize/2.0f;
                float h = 3.0f + (float)(i % 5);
                
                if (day <= 5) { // Awakening
                    DrawCylinder({x, 0, z}, 0.5f, 0.5f, h, 8, DARKGRAY);
                    DrawCube({x, h, z}, 2.0f, 0.5f, 2.0f, GRAY);
                } else if (day <= 10) { // Trials
                    DrawCylinder({x, 0, z}, 0.1f, 1.5f, h, 4, SKYBLUE); // Crystals
                } else { // Triumph
                    DrawCube({x, 0, z}, 3, h, 3, GOLD);
                    DrawCubeWires({x, 0, z}, 3, h, 3, WHITE);
                }
            }

            DrawCylinder(exitPoints[currentLevelName], 1.5f, 1.5f, 3.0f, 16, GOLD); 
        }

        for (const auto& s : currentLevelScriptures) {
            if (!s.found) {
                Vector3 animatedPos = s.position;
                animatedPos.y += sin(GetTime() * 2.0f) * 0.2f;
                DrawSphere(animatedPos, 0.4f, GOLD);
                DrawSphereWires(animatedPos, 0.4f, 8, 8, WHITE);
            }
        }

        for (const auto& p : projectiles) {
            if (p.active) {
                DrawSphere(p.position, p.radius, PURPLE);
                DrawSphereWires(p.position, p.radius, 6, 6, BLACK);
            }
        }
        
    EndShaderMode();
}

void LevelManager::AddProjectile(Vector3 pos, Vector3 vel, float rad, float life, bool fromP) {
    projectiles.push_back({pos, vel, rad, life, true, fromP});
}

Vector3 LevelManager::GetSpawnPoint(const std::string& levelName) {
    if (spawnPoints.count(levelName)) return spawnPoints[levelName];
    return (Vector3){0.0f, 0.5f, 0.0f};
}

Vector3 LevelManager::GetExitPosition(const std::string& levelName) {
    if (exitPoints.count(levelName)) return exitPoints[levelName];
    return (Vector3){0.0f, -100.0f, 0.0f};
}
