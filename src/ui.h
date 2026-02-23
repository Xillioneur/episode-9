#ifndef UI_H
#define UI_H

#include "raylib.h"
#include <string>

class HUD {
public:
    float scripturePopupTimer;
    std::string currentScriptureText;

    HUD();
    void Draw(float playerFaith, float playerMaxFaith, 
              float gloryBeamCD, float prayerBurstCD, float lightBladeCD, 
              int currentDay, int enemiesLeft, int missionType, int scripturesFound, int scripturesRequired);
    
    void ShowScripture(const std::string& text);
    void DrawScripturePopup();

    void DrawGameOver();
    void DrawVictory(int dayCompleted);
};

#endif // UI_H
