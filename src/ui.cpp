#include "ui.h"
#include "raylib.h"
#include "rlgl.h"
#include <string>
#include <cmath>

HUD::HUD() {
    scripturePopupTimer = 0.0f;
    currentScriptureText = "";
}

void HUD::ShowScripture(const std::string& text) {
    currentScriptureText = text;
    scripturePopupTimer = 5.0f; // Show for 5 seconds
}

void HUD::DrawScripturePopup() {
    if (scripturePopupTimer > 0) {
        float dt = GetFrameTime();
        scripturePopupTimer -= dt;

        float alpha = (scripturePopupTimer > 1.0f) ? 1.0f : scripturePopupTimer;
        Color boxColor = Fade(SKYBLUE, alpha * 0.8f);
        Color textColor = Fade(WHITE, alpha);

        int width = 600;
        int height = 100;
        int posX = GetScreenWidth() / 2 - width / 2;
        int posY = 100;

        DrawRectangle(posX, posY, width, height, boxColor);
        DrawRectangleLines(posX, posY, width, height, textColor);

        DrawText(currentScriptureText.c_str(), posX + 20, posY + 20, 20, textColor);
    }
}

void HUD::Draw(float playerFaith, float playerMaxFaith, 
              float gloryBeamCD, float prayerBurstCD, float lightBladeCD, 
              int currentDay, int enemiesLeft, int missionType, int scripturesFound, int scripturesRequired) {
    // --- Faith Bar (Top-left) ---
    float faithBarWidth = 200;
    float faithBarHeight = 20;
    float faithRatio = playerFaith / playerMaxFaith;
    DrawRectangle(10, 10, faithBarWidth, faithBarHeight, BLACK); 
    DrawRectangle(10, 10, (int)(faithBarWidth * faithRatio), faithBarHeight, GREEN); 
    DrawText(TextFormat("Faith: %.0f/%.0f", playerFaith, playerMaxFaith), 15, 12, 16, WHITE);

    // --- Mission Objective (Top-center) ---
    std::string objective = "";
    if (missionType == 0) objective = "Objective: Banish all Shadows of Doubt (" + std::to_string(enemiesLeft) + " left)";
    else if (missionType == 1) objective = "Objective: Restore the Light (" + std::to_string(scripturesFound) + "/" + std::to_string(scripturesRequired) + " Fragments)";
    else if (missionType == 2) objective = "Objective: Walk the Path of Faith to the Altar";
    else if (missionType == 3) objective = "Objective: Banish the Manifestation of Evil";

    int objWidth = MeasureText(objective.c_str(), 20);
    DrawRectangle(GetScreenWidth()/2 - objWidth/2 - 10, 10, objWidth + 20, 30, Fade(BLACK, 0.5f));
    DrawText(objective.c_str(), GetScreenWidth()/2 - objWidth/2, 15, 20, GOLD);

    // --- Ability Cooldowns (Bottom-left) ---
    int abilityTextY = GetScreenHeight() - 70;
    DrawText(TextFormat("LMB (Beam): %.1f", gloryBeamCD > 0 ? gloryBeamCD : 0), 10, abilityTextY, 16, gloryBeamCD > 0 ? RED : LIME);
    DrawText(TextFormat("RMB (Blade): %.1f", lightBladeCD > 0 ? lightBladeCD : 0), 10, abilityTextY + 20, 16, lightBladeCD > 0 ? RED : LIME);
    DrawText(TextFormat("E (Burst): %.1f", prayerBurstCD > 0 ? prayerBurstCD : 0), 10, abilityTextY + 40, 16, prayerBurstCD > 0 ? RED : LIME);

    // --- Day Counter (Top-right) ---
    std::string dayText = "Lenten Day: " + std::to_string(currentDay);
    DrawText(dayText.c_str(), GetScreenWidth() - MeasureText(dayText.c_str(), 20) - 10, 10, 20, BLACK);

    // --- Crosshair (Center) ---
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    float time = (float)GetTime();
    
    // Pulse effect
    float pulse = sinf(time * 4.0f) * 2.0f;
    float rotation = time * 90.0f; // Degrees per second

    // Draw Shadows/Outlines for contrast
    DrawCircle(centerX + 1, centerY + 1, 3, BLACK);
    
    // Rotating outer diamond (Divine geometry)
    rlPushMatrix();
    rlTranslatef((float)centerX, (float)centerY, 0);
    rlRotatef(rotation, 0, 0, 1);
    DrawRectangleLinesEx((Rectangle){ -8 - pulse/2, -8 - pulse/2, 16 + pulse, 16 + pulse }, 1.5f, Fade(GOLD, 0.4f));
    rlPopMatrix();

    // The Winged Cross
    int wingLen = 12;
    int wingOffset = 4;
    // Horizontal wings
    DrawLineEx((Vector2){ (float)centerX - wingLen - wingOffset, (float)centerY }, (Vector2){ (float)centerX - wingOffset, (float)centerY }, 2.0f, WHITE);
    DrawLineEx((Vector2){ (float)centerX + wingOffset, (float)centerY }, (Vector2){ (float)centerX + wingLen + wingOffset, (float)centerY }, 2.0f, WHITE);
    // Vertical wings
    DrawLineEx((Vector2){ (float)centerX, (float)centerY - wingLen - wingOffset }, (Vector2){ (float)centerX, (float)centerY - wingOffset }, 2.0f, WHITE);
    DrawLineEx((Vector2){ (float)centerX, (float)centerY + wingOffset }, (Vector2){ (float)centerX, (float)centerY + wingLen + wingOffset }, 2.0f, WHITE);

    // Inner Glow
    DrawCircle(centerX, centerY, 3.5f + pulse/4.0f, GOLD);
    DrawCircle(centerX, centerY, 1.5f, WHITE);
}

void HUD::DrawGameOver() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(MAROON, 0.8f));
    const char* title = "FAITH DEPLETED";
    const char* sub = "The shadows have momentarily clouded your path.";
    const char* hint = "Press R to Restart Day at Hub";
    DrawText(title, GetScreenWidth()/2 - MeasureText(title, 40)/2, GetScreenHeight()/2 - 60, 40, WHITE);
    DrawText(sub, GetScreenWidth()/2 - MeasureText(sub, 20)/2, GetScreenHeight()/2, 20, LIGHTGRAY);
    DrawText(hint, GetScreenWidth()/2 - MeasureText(hint, 20)/2, GetScreenHeight()/2 + 60, 20, GOLD);
}

void HUD::DrawVictory(int dayCompleted) {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(GOLD, 0.7f));
    const char* title = TextFormat("DAY %i TRIUMPHANT", dayCompleted);
    const char* sub = "Your light shines brighter than ever!";
    const char* hint = "Press SPACE to Return to Garden of Reflection";
    DrawText(title, GetScreenWidth()/2 - MeasureText(title, 40)/2, GetScreenHeight()/2 - 60, 40, DARKBLUE);
    DrawText(sub, GetScreenWidth()/2 - MeasureText(sub, 20)/2, GetScreenHeight()/2, 20, BLACK);
    DrawText(hint, GetScreenWidth()/2 - MeasureText(hint, 20)/2, GetScreenHeight()/2 + 60, 20, DARKPURPLE);
}
