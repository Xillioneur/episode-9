#include "raylib.h"
#include "raymath.h"
#include "camera3d.h" 
#include "player.h"   
#include "enemy.h"    
#include "glory_system.h" 
#include "particles3d.h" 
#include "level_manager.h" 
#include "audio_manager.h" 
#include "ui.h"        
#include "save.h"      
#include <vector>     
#include <memory>     
#include <algorithm>  

typedef enum GameScreen { TITLE = 0, GAMEPLAY, GAMEOVER, VICTORY, EXIT } GameScreen;

void InitializeNewGame(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                       LevelManager& levelManager, AudioManager& audioManager, int& currentDay);
void LoadGameState(const GameData& loadedData, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                   LevelManager& levelManager, AudioManager& audioManager, int& currentDay);
void LoadLevelEnemies(Level level, std::vector<std::unique_ptr<Enemy>>& enemies, AudioManager& audioManager, int currentDay, MissionType type);

const std::string DEFAULT_SAVE_FILE = "savegame.sav";

int main()
{
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Glory's Triumph 3D - Lenten Warfare");

    GameScreen currentScreen = TITLE;

    AudioManager audioManager;
    audioManager.LoadSounds();
    ParticleSystem particleSystem;
    LevelManager levelManager;
    GlorySystem glorySystem(particleSystem, audioManager);
    HUD hud;

    Player player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    int currentDay = 1;

    Camera3D_Custom customCamera;

    Shader lightingShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");
    int lightPosLoc = GetShaderLocation(lightingShader, "lightPos");
    int viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
    int ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");
    SetShaderValue(lightingShader, ambientColorLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f}, SHADER_UNIFORM_VEC4);

    Vector3 lightPosition = {0.0f, 10.0f, 0.0f};
    DisableCursor();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        switch (currentScreen)
        {
            case TITLE:
            {
                if (IsKeyPressed(KEY_N)) {
                    InitializeNewGame(player, enemies, levelManager, audioManager, currentDay);
                    customCamera.InitCamera(player.GetPosition());
                    customCamera.SetDistance(6.0f);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
                if (IsKeyPressed(KEY_L)) {
                    GameData loadedData = SaveSystem::LoadGame(DEFAULT_SAVE_FILE);
                    LoadGameState(loadedData, player, enemies, levelManager, audioManager, currentDay);
                    customCamera.InitCamera(player.GetPosition());
                    customCamera.SetDistance(6.0f);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
            } break;
            case GAMEPLAY:
            {
                player.Update(customCamera);
                customCamera.Update(player.GetPosition());
                audioManager.SetListenerPosition(customCamera.camera.position);
                audioManager.UpdateMusicStream();

                for (auto& enemy : enemies) {
                    enemy->Update(GetFrameTime(), player, levelManager, particleSystem); 
                }

                glorySystem.Update(GetFrameTime(), customCamera, player, enemies);
                particleSystem.Update(GetFrameTime());
                levelManager.UpdateCurrentLevel(GetFrameTime(), player);

                enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                             [](const std::unique_ptr<Enemy>& e){ return e->ReadyToRemove(); }),
                              enemies.end());

                lightPosition.x = 10.0f * sin(GetTime());
                lightPosition.z = 10.0f * cos(GetTime());

                if (player.faithMeter <= 0) { currentScreen = GAMEOVER; EnableCursor(); }

                Vector3 exitPos = levelManager.GetExitPosition();
                float distToExit = Vector3Distance(player.GetPosition(), exitPos);
                
                bool objectiveMet = false;
                if (levelManager.currentLevel == HUB) objectiveMet = true;
                else {
                    if (levelManager.currentMissionType == BANISH_DOUBT) objectiveMet = enemies.empty();
                    else if (levelManager.currentMissionType == RESTORE_LIGHT) objectiveMet = (levelManager.scripturesFoundInLevel >= levelManager.requiredScriptures);
                    else if (levelManager.currentMissionType == WALK_OF_FAITH) objectiveMet = true; 
                    else if (levelManager.currentMissionType == BOSS_TRIAL) objectiveMet = enemies.empty();
                }

                if (distToExit < 3.0f && IsKeyPressed(KEY_F)) {
                    if (levelManager.currentLevel == HUB) {
                        Level target = (Level)currentDay;
                        levelManager.GoToLevel(target); 
                        player.SetPosition(levelManager.GetSpawnPoint());
                        customCamera.InitCamera(player.GetPosition());
                        LoadLevelEnemies(target, enemies, audioManager, currentDay, levelManager.currentMissionType);
                    } else if (objectiveMet) {
                        currentScreen = VICTORY;
                        EnableCursor();
                    }
                }

                for (auto& s : levelManager.currentLevelScriptures) {
                    if (!s.found) {
                        float dist = Vector3Distance(player.GetPosition(), s.position);
                        if (dist < 2.0f && IsKeyPressed(KEY_F)) {
                            s.found = true;
                            levelManager.allFoundScriptureIDs.insert(s.id);
                            levelManager.scripturesFoundInLevel++;
                            hud.ShowScripture(s.text);
                            audioManager.PlaySFX("pickup", player.GetPosition());
                        }
                    }
                }

                if (IsKeyPressed(KEY_F5)) {
                    GameData d; d.playerPosition = player.GetPosition(); d.playerFaith = player.faithMeter; d.currentDay = currentDay;
                    for(const auto& e : enemies) if (e->IsBanished()) d.banishedEnemyPositions.push_back(e->spawnPosition); 
                    for (const auto& id : levelManager.allFoundScriptureIDs) d.collectedScriptureIDs.push_back(id);
                    SaveSystem::SaveGame(d, DEFAULT_SAVE_FILE);
                }
                if (IsKeyPressed(KEY_F9)) {
                    GameData loadedData = SaveSystem::LoadGame(DEFAULT_SAVE_FILE);
                    LoadGameState(loadedData, player, enemies, levelManager, audioManager, currentDay);
                    customCamera.InitCamera(player.GetPosition());
                }
            } break;
            case GAMEOVER:
                if (IsKeyPressed(KEY_R)) {
                    player.faithMeter = player.maxFaith;
                    levelManager.GoToLevel(HUB);
                    player.SetPosition(levelManager.GetSpawnPoint());
                    customCamera.InitCamera(player.GetPosition());
                    LoadLevelEnemies(HUB, enemies, audioManager, currentDay, levelManager.currentMissionType);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
                break;
            case VICTORY:
                if (IsKeyPressed(KEY_SPACE)) {
                    currentDay++; 
                    if (currentDay > 15) currentDay = 1; 
                    levelManager.GoToLevel(HUB);
                    player.SetPosition(levelManager.GetSpawnPoint());
                    customCamera.InitCamera(player.GetPosition());
                    LoadLevelEnemies(HUB, enemies, audioManager, currentDay, levelManager.currentMissionType);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
                break;
            default: break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (currentScreen) {
                case TITLE:
                    DrawText("GLORY'S TRIUMPH 3D", 190, 100, 40, DARKBLUE);
                    DrawText("Press N for New Game", 250, 200, 20, LIGHTGRAY);
                    DrawText("Press L for Load Game", 250, 230, 20, LIGHTGRAY);
                    break;
                case GAMEPLAY:
                case GAMEOVER:
                case VICTORY:
                    BeginMode3D(customCamera.camera);
                        SetShaderValue(lightingShader, lightPosLoc, (float*)&lightPosition, SHADER_UNIFORM_VEC3);
                        SetShaderValue(lightingShader, viewPosLoc, (float*)&customCamera.camera.position, SHADER_UNIFORM_VEC3);
                        levelManager.DrawCurrentLevel(lightingShader, lightPosLoc, viewPosLoc, lightPosition, customCamera.camera.position);
                        BeginShaderMode(lightingShader);
                            player.Draw(customCamera);
                            for (auto& enemy : enemies) enemy->Draw();
                        EndShaderMode();
                        particleSystem.Draw();
                        DrawSphere(lightPosition, 0.5f, YELLOW);
                        glorySystem.Draw(customCamera);
                    EndMode3D();
                    hud.Draw(player.faithMeter, player.maxFaith, glorySystem.gloryBeamTimer, glorySystem.prayerBurstTimer, glorySystem.lightBladeTimer,
                             currentDay, (int)enemies.size(), (int)levelManager.currentMissionType, levelManager.scripturesFoundInLevel, levelManager.requiredScriptures);
                    hud.DrawScripturePopup();
                    if (currentScreen == GAMEOVER) hud.DrawGameOver();
                    if (currentScreen == VICTORY) hud.DrawVictory(currentDay);
                    break;
                default: break;
            }
        EndDrawing();
    }
    UnloadShader(lightingShader);
    CloseWindow();
    return 0;
}

void LoadLevelEnemies(Level level, std::vector<std::unique_ptr<Enemy>>& enemies, AudioManager& audioManager, int currentDay, MissionType type) {
    enemies.clear();
    if (level == HUB) {
        enemies.push_back(std::make_unique<ShadowDrone>((Vector3){5.0f, 2.0f, 5.0f}, audioManager));
        enemies.push_back(std::make_unique<ShadowDrone>((Vector3){-5.0f, 3.0f, -5.0f}, audioManager));
    } else {
        if (type == BOSS_TRIAL) {
            BossType bt = PRIDE;
            if (currentDay == 10) bt = DESPAIR;
            else if (currentDay == 15) bt = DEATH;
            enemies.push_back(std::make_unique<Boss>((Vector3){0, 0, 30.0f}, bt, audioManager));
        } else {
            int enemyCount = 3 + (currentDay); 
            for (int i = 0; i < enemyCount; i++) {
                float angle = (float)i * (360.0f / enemyCount) * DEG2RAD;
                float r = 15.0f + (float)currentDay;
                Vector3 pos = { cosf(angle) * r, 0.5f, sinf(angle) * r + 20.0f };
                if (i % 3 == 0 && currentDay >= 3) enemies.push_back(std::make_unique<TemptationBeast>(pos, audioManager));
                else if (i % 2 == 0) enemies.push_back(std::make_unique<Whisperer>(pos, audioManager));
                else enemies.push_back(std::make_unique<ShadowDrone>((Vector3){pos.x, 4.0f, pos.z}, audioManager));
            }
        }
    }
}

void InitializeNewGame(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies, LevelManager& levelManager, AudioManager& audioManager, int& currentDay) {
    player = Player(); levelManager.allFoundScriptureIDs.clear(); currentDay = 1; 
    levelManager.GoToLevel(HUB);
    player.SetPosition(levelManager.GetSpawnPoint());
    LoadLevelEnemies(HUB, enemies, audioManager, currentDay, levelManager.currentMissionType);
}

void LoadGameState(const GameData& loadedData, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies, LevelManager& levelManager, AudioManager& audioManager, int& currentDay) {
    player.SetPosition(loadedData.playerPosition); player.faithMeter = loadedData.playerFaith; currentDay = loadedData.currentDay;
    levelManager.allFoundScriptureIDs.clear();
    for (const auto& id : loadedData.collectedScriptureIDs) levelManager.allFoundScriptureIDs.insert(id);
    levelManager.GoToLevel(HUB); 
    LoadLevelEnemies(HUB, enemies, audioManager, currentDay, levelManager.currentMissionType);
}
