#include "ui.h"
#include "raylib.h"
#include <string>

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

        // Simple word wrap simulation (could be improved)
        DrawText(currentScriptureText.c_str(), posX + 20, posY + 20, 20, textColor);
    }
}

void HUD::Draw(float playerFaith, float playerMaxFaith, 
              float gloryBeamCD, float prayerBurstCD, float lightBladeCD, 
              int currentDay, int enemiesLeft) {
    // --- Faith Bar (Top-left) ---
    float faithBarWidth = 200;
    float faithBarHeight = 20;
    float faithRatio = playerFaith / playerMaxFaith;
    DrawRectangle(10, 10, faithBarWidth, faithBarHeight, BLACK); // Background
    DrawRectangle(10, 10, (int)(faithBarWidth * faithRatio), faithBarHeight, GREEN); // Fill
    DrawText(TextFormat("Faith: %.0f/%.0f", playerFaith, playerMaxFaith), 15, 12, 16, WHITE);

    // --- Ability Cooldowns (Bottom-left) ---
    int abilityTextY = GetScreenHeight() - 70;
    DrawText(TextFormat("LMB (Beam): %.1f", gloryBeamCD > 0 ? gloryBeamCD : 0), 10, abilityTextY, 16, gloryBeamCD > 0 ? RED : LIME);
    DrawText(TextFormat("RMB (Blade): %.1f", lightBladeCD > 0 ? lightBladeCD : 0), 10, abilityTextY + 20, 16, lightBladeCD > 0 ? RED : LIME);
    DrawText(TextFormat("E (Burst): %.1f", prayerBurstCD > 0 ? prayerBurstCD : 0), 10, abilityTextY + 40, 16, prayerBurstCD > 0 ? RED : LIME);

    // --- Day Counter (Top-right) ---
    std::string dayText = "Day: " + std::to_string(currentDay);
    DrawText(dayText.c_str(), GetScreenWidth() - MeasureText(dayText.c_str(), 20) - 10, 10, 20, BLACK);

    // --- Enemies Left (Top-right, below Day Counter) ---
    std::string enemiesText = "Enemies: " + std::to_string(enemiesLeft);
    DrawText(enemiesText.c_str(), GetScreenWidth() - MeasureText(enemiesText.c_str(), 18) - 10, 35, 18, BLACK);

    // --- Crosshair (Center) ---
    int centerX = GetScreenWidth() / 2;
    int centerY = GetScreenHeight() / 2;
    int crosshairSize = 10;
    DrawCircle(centerX, centerY, 3, Fade(GOLD, 0.6f));
    DrawLine(centerX - crosshairSize, centerY, centerX + crosshairSize, centerY, Fade(WHITE, 0.4f));
    DrawLine(centerX, centerY - crosshairSize, centerX, centerY + crosshairSize, Fade(WHITE, 0.4f));
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
