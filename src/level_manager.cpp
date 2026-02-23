#include "level_manager.h"
#include "raylib.h"
#include "raymath.h" 
#include "player.h"

LevelManager::LevelManager() {
    currentLevel = HUB;
    currentLevelName = "Garden of Reflection";
    currentMissionType = WALK_OF_FAITH;
    requiredScriptures = 0;
    scripturesFoundInLevel = 0;

    spawnPoints[HUB] = {0, 0.5f, 0};
    exitPoints[HUB] = {0, 0, -15.0f};

    for (int i = 1; i <= 15; i++) {
        Level l = (Level)i;
        spawnPoints[l] = {0, 0.5f, 0};
        if (i % 2 == 0) exitPoints[l] = {20.0f, 0, 50.0f};
        else exitPoints[l] = {-20.0f, 0, 60.0f};
        if (i % 5 == 0) exitPoints[l] = {0, 0, 40.0f}; 
    }
}

void LevelManager::GoToLevel(Level level) {
    currentLevel = level;
    currentLevelScriptures.clear();
    currentLevelDecorations.clear();
    projectiles.clear();
    scripturesFoundInLevel = 0;

    if (level == HUB) {
        currentLevelName = "Garden of Reflection";
        currentMissionType = WALK_OF_FAITH;
        currentLevelScriptures.push_back({"hub_1", "John 1:5 - The light shines in the darkness.", {15.0f, 1.0f, -10.0f}, false});
        
        // Static Hub Decorations
        for(int i=0; i<20; i++) {
            float a = i * 18.0f * DEG2RAD;
            currentLevelDecorations.push_back({{cosf(a)*60, 0, sinf(a)*60}, 5.0f, 0.5f, 0}); // Type 0: Tree
        }
    } else {
        int day = (int)level;
        currentLevelName = "Lenten Day " + std::to_string(day);
        
        if (day == 5 || day == 10 || day == 15) currentMissionType = BOSS_TRIAL;
        else if (day % 4 == 1) currentMissionType = BANISH_DOUBT;
        else if (day % 4 == 2) currentMissionType = RESTORE_LIGHT;
        else currentMissionType = WALK_OF_FAITH;

        std::string text = "A Sacred Verse reveals itself.";
        if (day == 1) text = "Psalm 27:1 - The LORD is my light and my salvation.";
        else if (day == 15) text = "Matthew 28:6 - He is not here: for he is risen!";

        if (currentMissionType == RESTORE_LIGHT) {
            requiredScriptures = 3;
            for(int i=0; i<3; i++) {
                float x = (i == 0) ? 15 : ((i == 1) ? -15 : 0);
                float z = (i == 2) ? 45 : 25;
                currentLevelScriptures.push_back({"d"+std::to_string(day)+"_"+std::to_string(i), text, {x, 1.0f, z}, false});
            }
        } else {
            requiredScriptures = 1;
            currentLevelScriptures.push_back({"d"+std::to_string(day)+"_m", text, {0, 1.0f, 20.0f}, false});
        }

        // Generate static level decorations based on day
        float levelSize = 100.0f + (day * 5.0f);
        if (day == 2) {
            for(int x=-40; x<=40; x+=20) for(int z=10; z<=80; z+=20) 
                currentLevelDecorations.push_back({{(float)x, 0, (float)z}, 8.0f, 1.5f, 2}); // Type 2: Pillar
        }
        else if (day == 3) {
            for(int i=0; i<40; i++) {
                float rx = (float)GetRandomValue(-50, 50);
                float rz = (float)GetRandomValue(10, 90);
                currentLevelDecorations.push_back({{rx, 0, rz}, 4.0f, 0.2f, 0}); // Type 0: Tree
            }
        }
        else if (day == 4) {
            for(int i=0; i<20; i++) 
                currentLevelDecorations.push_back({{(i%2==0?15.0f:-15.0f), 0, (float)i*5}, 10.0f, 0.1f, 1}); // Type 1: Crystal/Shard
        }
        else if (day == 6) {
            for(int i=0; i<30; i++)
                currentLevelDecorations.push_back({{(float)GetRandomValue(-60,60), 1, (float)GetRandomValue(10,90)}, 2.0f, 4.0f, 2});
        }
        // ... add more as needed for other days
    }

    for (auto& s : currentLevelScriptures) if (allFoundScriptureIDs.count(s.id)) s.found = true;
}

void LevelManager::UnloadLevel() {
    currentLevelName = "";
    currentLevelScriptures.clear();
    currentLevelDecorations.clear();
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

        int day = (int)currentLevel;
        float levelSize = 100.0f + (day * 5.0f);

        if (currentLevel == HUB) {
            DrawCube({0, -0.5f, 0}, 200, 1, 200, DARKGREEN);
            DrawCylinder(exitPoints[HUB], 2.0f, 2.0f, 4.0f, 16, PURPLE); 
        }
        else {
            Color groundColor = ColorFromHSV((float)((day * 24) % 360), 0.6f, 0.4f);
            
            // Handcrafted visual specifics per day
            switch(day) {
                case 1: DrawCube({0, -0.5f, 40}, 40, 1, 100, MAROON); 
                        DrawCube({-20, 5, 40}, 2, 10, 100, DARKGRAY);
                        DrawCube({20, 5, 40}, 2, 10, 100, DARKGRAY);
                        break;
                case 11: for(int i=0; i<8; i++) DrawCube({sinf(i)*15, (float)i, (float)i*12}, 15, 1, 15, DARKPURPLE); break;
                case 12: for(int i=0; i<20; i++) DrawCube({0, (float)i*0.5f, (float)i*4}, 20, 1, 5, WHITE); break;
                default: DrawCube({0, -0.5f, levelSize/2}, levelSize, 1.0f, levelSize, groundColor); break;
            }
            DrawCylinder(exitPoints[currentLevel], 1.5f, 1.5f, 3.0f, 16, GOLD); 
        }

        // Draw STORED Decorations (Static)
        for (const auto& d : currentLevelDecorations) {
            if (d.type == 0) { // Tree
                DrawCylinder(d.position, 0.2f, d.radius, d.height, 5, BROWN);
                DrawSphere({d.position.x, d.height + 0.5f, d.position.z}, 1.5f, GREEN);
            } else if (d.type == 1) { // Crystal
                DrawCylinder(d.position, 0.1f, d.radius, d.height, 4, WHITE);
            } else if (d.type == 2) { // Pillar
                DrawCylinder(d.position, d.radius, d.radius, d.height, 8, GRAY);
            }
        }

        for (const auto& s : currentLevelScriptures) {
            if (!s.found) {
                Vector3 p = s.position; p.y += sin(GetTime() * 2.0f) * 0.2f;
                DrawSphere(p, 0.4f, GOLD);
                DrawSphereWires(p, 0.4f, 8, 8, WHITE);
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

Vector3 LevelManager::GetSpawnPoint() { return spawnPoints[currentLevel]; }
Vector3 LevelManager::GetExitPosition() { return exitPoints[currentLevel]; }
