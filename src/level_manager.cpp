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
        float angle = (float)i * 0.8f;
        float dist = 40.0f + (i * 4.0f);
        exitPoints[l] = {sinf(angle) * 20.0f, 0, dist};
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
        for(int i=0; i<30; i++) {
            float a = i * 12.0f * DEG2RAD;
            currentLevelDecorations.push_back({{cosf(a)*65, 0, sinf(a)*65}, 6.0f, 0.6f, 0}); 
        }
    } else {
        int day = (int)level;
        currentLevelName = "Lenten Day " + std::to_string(day);
        if (day == 5 || day == 10 || day == 15) currentMissionType = BOSS_TRIAL;
        else if (day % 4 == 1) currentMissionType = BANISH_DOUBT;
        else if (day % 4 == 2) currentMissionType = RESTORE_LIGHT;
        else currentMissionType = WALK_OF_FAITH;

        std::string text = "A Divine Revelation.";
        if (day == 1) text = "Psalm 27:1 - The LORD is my light.";
        else if (day == 15) text = "Victory: He is Risen!";

        if (currentMissionType == RESTORE_LIGHT) {
            requiredScriptures = 3;
            for(int i=0; i<3; i++) {
                float rx = (float)((i-1)*20);
                float rz = 20.0f + (float)i*15.0f;
                currentLevelScriptures.push_back({"d"+std::to_string(day)+"_"+std::to_string(i), text, {rx, 1.0f, rz}, false});
            }
        } else {
            requiredScriptures = 1;
            currentLevelScriptures.push_back({"d"+std::to_string(day)+"_m", text, {0, 1.0f, 30.0f}, false});
        }

        // Static Decorations per Level
        for (int i = 0; i < 8 + day; i++) {
            float rx = sinf(i * 1.5f + day) * 30.0f;
            float rz = cosf(i * 2.0f + day) * 30.0f + 40.0f;
            currentLevelDecorations.push_back({{rx, 0, rz}, 3.0f + (i%5), 0.5f + (float)(i%3)*0.2f, 2});
        }
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
        if (currentLevel == HUB) {
            for(int x=-80; x<=80; x+=10) for(int z=-80; z<=80; z+=10) {
                DrawCube({(float)x, -0.5f, (float)z}, 10, 1, 10, ((x+z)/10 % 2 == 0) ? DARKGREEN : GREEN);
            }
            DrawCylinder(exitPoints[HUB], 2.0f, 2.0f, 4.0f, 16, PURPLE); 
        }
        else {
            Color ground = ColorFromHSV((float)((day * 24) % 360), 0.5f, 0.3f);
            
            // --- 15 UNIQUE HANDCRAFTED LAYOUTS ---
            switch(day) {
                case 1: DrawCube({0, -0.5f, 50}, 40, 1, 120, MAROON); break;
                case 2: DrawCube({0, -0.5f, 50}, 100, 1, 100, DARKBLUE); break;
                case 3: DrawCube({0, -0.5f, 50}, 120, 1, 120, DARKGREEN); break;
                case 4: DrawCube({0, -0.5f, 50}, 60, 1, 150, SKYBLUE); break;
                case 5: DrawCube({0, -0.5f, 30}, 100, 1, 100, PURPLE); break;
                case 6: DrawCube({0, -0.5f, 60}, 150, 1, 150, GOLD); break;
                case 7: DrawCube({0, -0.5f, 80}, 20, 1, 200, BLACK); break;
                case 8: DrawCube({0, -0.5f, 40}, 100, 1, 100, DARKGRAY); break;
                case 9: DrawCube({0, -0.5f, 50}, 120, 1, 120, BEIGE); break;
                case 10: DrawCube({0, -0.5f, 30}, 150, 1, 150, BLUE); break;
                case 11: for(int i=0; i<10; i++) DrawCube({0, (float)i*0.5f, (float)i*10}, 30, 1, 15, ground); break;
                case 12: for(int i=0; i<10; i++) DrawCube({sinf(i)*20, (float)i*2, (float)i*15}, 20, 1, 20, ground); break;
                case 13: DrawCube({0, -0.5f, 100}, 200, 1, 300, WHITE); break;
                case 14: DrawCube({0, -0.5f, 100}, 50, 1, 400, GOLD); break;
                case 15: DrawCube({0, -0.5f, 40}, 200, 1, 200, WHITE); break;
                default: DrawCube({0, -0.5f, 50}, 100, 1, 100, ground); break;
            }
            DrawCylinder(exitPoints[currentLevel], 1.5f, 1.5f, 3.0f, 16, GOLD); 
        }

        for (const auto& d : currentLevelDecorations) {
            if (d.type == 0) { 
                DrawCylinder(d.position, 0.1f, d.radius, d.height, 6, BROWN);
                DrawSphere({d.position.x, d.height, d.position.z}, d.radius*3.0f, DARKGREEN);
            } else if (d.type == 2) { 
                DrawCylinder(d.position, d.radius, d.radius, d.height, 8, LIGHTGRAY);
                DrawCube({d.position.x, d.height, d.position.z}, d.radius*2.5f, 0.5f, d.radius*2.5f, GRAY);
            }
        }

        for (const auto& s : currentLevelScriptures) {
            if (!s.found) {
                Vector3 p = s.position; p.y += sin(GetTime() * 2.0f) * 0.2f;
                DrawSphere(p, 0.4f, GOLD);
                DrawSphereWires(p, 0.5f, 8, 8, WHITE);
            }
        }
        for (const auto& p : projectiles) {
            if (p.active) {
                DrawSphere(p.position, p.radius, PURPLE);
                DrawSphereWires(p.position, p.radius * 1.2f, 6, 6, WHITE);
            }
        }
    EndShaderMode();
}

void LevelManager::AddProjectile(Vector3 pos, Vector3 vel, float rad, float life, bool fromP) {
    projectiles.push_back({pos, vel, rad, life, true, fromP});
}

Vector3 LevelManager::GetSpawnPoint() { return spawnPoints[currentLevel]; }
Vector3 LevelManager::GetExitPosition() { return exitPoints[currentLevel]; }
