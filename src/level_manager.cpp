#include "level_manager.h"
#include "raylib.h"
#include "raymath.h" 
#include "player.h"

LevelManager::LevelManager() {
    currentLevelName = "";

    spawnPoints["Hub"] = (Vector3){0.0f, 0.5f, 0.0f}; 
    exitPoints["Hub"] = (Vector3){0.0f, 0.0f, -15.0f}; 

    for (int i = 1; i <= 15; i++) {
        std::string dayName = "Day" + std::to_string(i);
        spawnPoints[dayName] = (Vector3){0.0f, 0.5f, 0.0f};
        exitPoints[dayName] = (Vector3){0.0f, 0.0f, 40.0f + (i * 2.0f)}; 
    }
}

void LevelManager::LoadLevel(const std::string& levelName) {
    currentLevelName = levelName;
    currentLevelScriptures.clear();
    projectiles.clear();

    if (levelName == "Hub") {
        currentLevelScriptures.push_back({"hub_1", "John 1:5 - The light shines in the darkness.", {15.0f, 1.0f, -10.0f}, false});
    } else {
        int day = 1;
        if (levelName.length() > 3) day = std::stoi(levelName.substr(3));

        if (day == 1) currentLevelScriptures.push_back({"day1_1", "Psalm 27:1 - The LORD is my light.", {0.0f, 1.0f, 25.0f}, false});
        else if (day == 2) currentLevelScriptures.push_back({"day2_1", "Matthew 5:14 - Ye are the light of the world.", {10.0f, 1.0f, 10.0f}, false});
        else if (day == 3) currentLevelScriptures.push_back({"day3_1", "Ephesians 5:8 - Walk as children of light.", {-10.0f, 1.0f, 20.0f}, false});
        else if (day == 4) currentLevelScriptures.push_back({"day4_1", "1 John 1:7 - Walk in the light.", {5.0f, 1.0f, 30.0f}, false});
        else if (day >= 5) currentLevelScriptures.push_back({"day_gen", "Lenten Virtue: Perseverance and Faith.", {0.0f, 1.0f, 15.0f + day}, false});
    }

    for (auto& s : currentLevelScriptures) {
        if (allFoundScriptureIDs.count(s.id)) s.found = true;
    }

    TraceLog(LOG_INFO, "LEVEL: Loading level %s", levelName.c_str());
}

void LevelManager::UnloadLevel() {
    TraceLog(LOG_INFO, "LEVEL: Unloading level %s", currentLevelName.c_str());
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
            DrawCube((Vector3){0.0f, -0.5f, 0.0f}, 100.0f, 1.0f, 100.0f, GREEN); 
            // Hub Decorations
            for (int i = 0; i < 8; i++) {
                float x = sinf(i * 1.0f) * 20.0f;
                float z = cosf(i * 1.0f) * 20.0f;
                DrawCube((Vector3){x, 1.5f, z}, 2.0f, 3.0f, 2.0f, DARKGRAY);
                DrawSphere((Vector3){x, 3.5f, z}, 1.5f, DARKGREEN); // Simple "Trees"
            }
            DrawCylinder(exitPoints["Hub"], 2.0f, 2.0f, 4.0f, 16, PURPLE); 
        }
        else {
            int day = std::stoi(currentLevelName.substr(3));
            Color groundColor = ColorFromHSV((float)(day % 15) / 15.0f * 360.0f, 0.6f, 0.4f);
            float levelSize = 60.0f + (day * 5.0f);
            DrawCube((Vector3){0.0f, -0.5f, levelSize/2.0f}, levelSize, 1.0f, levelSize, groundColor); 
            
            for (int i = 0; i < 5 + day; i++) {
                float x = sinf(i * 1.5f + day) * (levelSize * 0.4f);
                float z = cosf(i * 2.0f + day) * (levelSize * 0.4f) + levelSize/2.0f;
                float h = 2.0f + (float)(i % 4);
                // "Realistic" Pillar/Ruins
                DrawCube((Vector3){x, h/2.0f, z}, 4.0f, h, 4.0f, GRAY);
                DrawCubeWires((Vector3){x, h/2.0f, z}, 4.0f, h, 4.0f, DARKGRAY);
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
