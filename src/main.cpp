#include "raylib.h"
#include "raymath.h"
#include "camera3d.h" // Include our custom camera header
#include "player.h"   // Include our player header
#include "enemy.h"    // Include our enemy header
#include "glory_system.h" // Include our glory system header
#include "particles3d.h" // Include our particle system header
#include "level_manager.h" // Include our level manager header
#include "audio_manager.h" // Include our audio manager header
#include "ui.h"        // Include our UI manager header
#include "save.h"      // Include our save system header
#include <vector>     // For std::vector
#include <memory>     // For std::unique_ptr
#include <algorithm>  // For std::remove_if

// Define game states
typedef enum GameScreen { TITLE = 0, GAMEPLAY, GAMEOVER, VICTORY, EXIT } GameScreen;

// --- Forward Declarations for Game State Management ---
void InitializeNewGame(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                       LevelManager& levelManager, AudioManager& audioManager, int& currentDay);
void LoadGameState(const GameData& loadedData, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                   LevelManager& levelManager, AudioManager& audioManager, int& currentDay);
void LoadLevelEnemies(const std::string& levelName, std::vector<std::unique_ptr<Enemy>>& enemies, AudioManager& audioManager, int currentDay);

// --- Constants ---
const std::string DEFAULT_SAVE_FILE = "savegame.sav";

int main()
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Glory's Triumph 3D - Lenten Warfare");

    GameScreen currentScreen = TITLE;

    // Core Game Systems
    AudioManager audioManager;
    audioManager.LoadSounds();
    ParticleSystem particleSystem;
    LevelManager levelManager;
    GlorySystem glorySystem(particleSystem, audioManager);
    HUD hud;

    // Player, Enemies, Day
    Player player;
    std::vector<std::unique_ptr<Enemy>> enemies;
    int currentDay = 1;

    // Custom Camera initialization
    Camera3D_Custom customCamera;

    // Load lighting shader
    Shader lightingShader = LoadShader("shaders/lighting.vs", "shaders/lighting.fs");
    int lightPosLoc = GetShaderLocation(lightingShader, "lightPos");
    int viewPosLoc = GetShaderLocation(lightingShader, "viewPos");
    int ambientColorLoc = GetShaderLocation(lightingShader, "ambientColor");

    SetShaderValue(lightingShader, ambientColorLoc, (float[4]){0.2f, 0.2f, 0.2f, 1.0f}, SHADER_UNIFORM_VEC4);

    // Dynamic light position
    Vector3 lightPosition = {0.0f, 10.0f, 0.0f};

    // Lock mouse for camera control initially
    DisableCursor();

    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())
    {
        // Update
        //----------------------------------------------------------------------------------
        switch (currentScreen)
        {
            case TITLE:
            {
                if (IsKeyPressed(KEY_N)) // N for New Game
                {
                    InitializeNewGame(player, enemies, levelManager, audioManager, currentDay);
                    customCamera.InitCamera(player.GetPosition());
                    customCamera.SetDistance(6.0f);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
                if (IsKeyPressed(KEY_L)) // L for Load Game
                {
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

                // Updated GlorySystem Update call
                glorySystem.Update(GetFrameTime(), customCamera, player, enemies);
                
                particleSystem.Update(GetFrameTime());
                levelManager.UpdateCurrentLevel(GetFrameTime(), player);

                enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                                             [](const std::unique_ptr<Enemy>& e){ return e->ReadyToRemove(); }),
                              enemies.end());

                lightPosition.x = 10.0f * sin(GetTime());
                lightPosition.z = 10.0f * cos(GetTime());

                if (player.faithMeter <= 0) {
                    currentScreen = GAMEOVER;
                    EnableCursor(); 
                }

                Vector3 exitPos = levelManager.GetExitPosition(levelManager.currentLevelName);
                float distToExit = Vector3Distance(player.GetPosition(), exitPos);
                if (distToExit < 3.0f && IsKeyPressed(KEY_F)) {
                    if (levelManager.currentLevelName == "Hub") {
                        std::string targetDay = "Day" + std::to_string(currentDay);
                        levelManager.LoadLevel(targetDay); 
                        player.SetPosition(levelManager.GetSpawnPoint(targetDay));
                        customCamera.InitCamera(player.GetPosition());
                        LoadLevelEnemies(targetDay, enemies, audioManager, currentDay);
                    } else {
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
                            hud.ShowScripture(s.text);
                            audioManager.PlaySFX("pickup", player.GetPosition());
                        }
                    }
                }

                if (IsKeyPressed(KEY_F5)) {
                    GameData dataToSave;
                    dataToSave.playerPosition = player.GetPosition();
                    dataToSave.playerFaith = player.faithMeter;
                    dataToSave.currentDay = currentDay;
                    for(const auto& enemy_ptr : enemies) {
                        if (enemy_ptr->IsBanished()) {
                            dataToSave.banishedEnemyPositions.push_back(enemy_ptr->spawnPosition); 
                        }
                    }
                    for (const auto& id : levelManager.allFoundScriptureIDs) {
                        dataToSave.collectedScriptureIDs.push_back(id);
                    }
                    SaveSystem::SaveGame(dataToSave, DEFAULT_SAVE_FILE);
                }
                if (IsKeyPressed(KEY_F9)) {
                    GameData loadedData = SaveSystem::LoadGame(DEFAULT_SAVE_FILE);
                    LoadGameState(loadedData, player, enemies, levelManager, audioManager, currentDay);
                    customCamera.InitCamera(player.GetPosition());
                    customCamera.SetDistance(6.0f);
                }

            } break;
            case GAMEOVER:
            {
                if (IsKeyPressed(KEY_R)) {
                    player.faithMeter = player.maxFaith;
                    levelManager.LoadLevel("Hub");
                    player.SetPosition(levelManager.GetSpawnPoint("Hub"));
                    customCamera.InitCamera(player.GetPosition());
                    LoadLevelEnemies("Hub", enemies, audioManager, currentDay);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
            } break;
            case VICTORY:
            {
                if (IsKeyPressed(KEY_SPACE)) {
                    currentDay++; 
                    if (currentDay > 15) currentDay = 1; 
                    
                    levelManager.LoadLevel("Hub");
                    player.SetPosition(levelManager.GetSpawnPoint("Hub"));
                    customCamera.InitCamera(player.GetPosition());
                    LoadLevelEnemies("Hub", enemies, audioManager, currentDay);
                    currentScreen = GAMEPLAY;
                    DisableCursor();
                }
            } break;
            default: break;
        }
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            switch (currentScreen)
            {
                case TITLE:
                {
                    DrawText("GLORY'S TRIUMPH 3D", 190, 100, 40, DARKBLUE);
                    DrawText("Press N for New Game", 250, 200, 20, LIGHTGRAY);
                    DrawText("Press L for Load Game", 250, 230, 20, LIGHTGRAY);
                } break;
                case GAMEPLAY:
                case GAMEOVER:
                case VICTORY:
                {
                    BeginMode3D(customCamera.camera);

                        SetShaderValue(lightingShader, lightPosLoc, (float*)&lightPosition, SHADER_UNIFORM_VEC3);
                        SetShaderValue(lightingShader, viewPosLoc, (float*)&customCamera.camera.position, SHADER_UNIFORM_VEC3);
                        
                        levelManager.DrawCurrentLevel(lightingShader, lightPosLoc, viewPosLoc, lightPosition, customCamera.camera.position);
                        
                        BeginShaderMode(lightingShader);
                            player.Draw(customCamera);
                            for (auto& enemy : enemies) {
                                enemy->Draw();
                            }
                        EndShaderMode();

                        particleSystem.Draw();
                        DrawSphere(lightPosition, 0.5f, YELLOW);
                        glorySystem.Draw(customCamera);

                    EndMode3D();

                    hud.Draw(player.faithMeter, player.maxFaith,
                             glorySystem.gloryBeamTimer, glorySystem.prayerBurstTimer, glorySystem.lightBladeTimer,
                             currentDay, (int)enemies.size());
                    
                    hud.DrawScripturePopup();

                    DrawText(TextFormat("Level: %s", levelManager.currentLevelName.c_str()), 10, GetScreenHeight() - 25, 18, DARKGRAY);
                    
                    if (currentScreen == GAMEOVER) hud.DrawGameOver();
                    if (currentScreen == VICTORY) hud.DrawVictory(currentDay);

                } break;
                default: break;
            }

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    UnloadShader(lightingShader);
    CloseWindow();

    return 0;
}

// --- Helper Functions Implementation ---

void LoadLevelEnemies(const std::string& levelName, std::vector<std::unique_ptr<Enemy>>& enemies, AudioManager& audioManager, int currentDay) {
    enemies.clear();
    if (levelName == "Hub") {
        enemies.push_back(std::make_unique<ShadowDrone>((Vector3){5.0f, 2.0f, 5.0f}, audioManager));
        enemies.push_back(std::make_unique<ShadowDrone>((Vector3){-5.0f, 3.0f, -5.0f}, audioManager));
        enemies.push_back(std::make_unique<ShadowDrone>((Vector3){0.0f, 4.0f, -8.0f}, audioManager));
    } else {
        int enemyCount = 3 + (currentDay * 2); 
        for (int i = 0; i < enemyCount; i++) {
            float angle = (float)i * (360.0f / enemyCount) * DEG2RAD;
            float spawnRadius = 15.0f + (float)currentDay;
            Vector3 pos = { cosf(angle) * spawnRadius, 0.5f, sinf(angle) * spawnRadius + 30.0f };
            
            if (i % 3 == 0 && currentDay >= 3) {
                enemies.push_back(std::make_unique<TemptationBeast>(pos, audioManager));
            }
            else if (i % 2 == 0) {
                enemies.push_back(std::make_unique<Whisperer>(pos, audioManager));
            }
            else {
                enemies.push_back(std::make_unique<ShadowDrone>((Vector3){pos.x, 4.0f + (float)(i%3), pos.z}, audioManager));
            }
        }
    }
}

void InitializeNewGame(Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                       LevelManager& levelManager, AudioManager& audioManager, int& currentDay) {
    TraceLog(LOG_INFO, "GAME: Initializing New Game...");
    player = Player(); 
    levelManager.allFoundScriptureIDs.clear();
    currentDay = 1; 
    levelManager.LoadLevel("Hub");
    player.SetPosition(levelManager.GetSpawnPoint(levelManager.currentLevelName));
    LoadLevelEnemies("Hub", enemies, audioManager, currentDay);
    TraceLog(LOG_INFO, "GAME: New Game initialized.");
}

void LoadGameState(const GameData& loadedData, Player& player, std::vector<std::unique_ptr<Enemy>>& enemies,
                   LevelManager& levelManager, AudioManager& audioManager, int& currentDay) {
    TraceLog(LOG_INFO, "GAME: Loading Game State...");
    player.SetPosition(loadedData.playerPosition);
    player.faithMeter = loadedData.playerFaith;
    currentDay = loadedData.currentDay;
    
    levelManager.allFoundScriptureIDs.clear();
    for (const auto& id : loadedData.collectedScriptureIDs) {
        levelManager.allFoundScriptureIDs.insert(id);
    }

    levelManager.LoadLevel("Hub"); 
    LoadLevelEnemies("Hub", enemies, audioManager, currentDay);
    TraceLog(LOG_INFO, "GAME: Game State loaded.");
}
