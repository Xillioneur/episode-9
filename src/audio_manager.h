#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include "raylib.h"
#include "raymath.h"
#include <string>
#include <map>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    void LoadSounds();
    void UnloadSounds();

    void PlayMusic(const std::string& musicName);
    void StopMusic();
    void UpdateMusicStream(); // Raylib requires music stream update

    void PlaySFX(const std::string& sfxName, Vector3 position, float volume = 1.0f, float pitch = 1.0f);
    void SetListenerPosition(Vector3 position);

private:
    std::map<std::string, Music> musicMap;
    std::map<std::string, Sound> sfxMap;

    Music currentMusic; // Track currently playing music
};

#endif // AUDIO_MANAGER_H
