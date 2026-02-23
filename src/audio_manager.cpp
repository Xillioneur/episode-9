#include "audio_manager.h"
#include <iostream> // For logging

// Private member to store listener position for 3D audio simulation
static Vector3 listenerPosition = { 0.0f, 0.0f, 0.0f };

// Constructor
AudioManager::AudioManager() {
    InitAudioDevice();
    SetMasterVolume(0.5f); // Default master volume
    currentMusic = { 0 }; // Initialize current music to null/empty
    TraceLog(LOG_INFO, "AUDIO: Audio device initialized.");
}

// Destructor
AudioManager::~AudioManager() {
    UnloadSounds();
    CloseAudioDevice();
    TraceLog(LOG_INFO, "AUDIO: Audio device closed.");
}

// Load all game sounds and music
void AudioManager::LoadSounds() {
    // --- Placeholder for actual sound/music loading ---
    // Example:
    // sfxMap["glory_hit"] = LoadSound("resources/audio/glory_hit.wav");
    // musicMap["background_theme"] = LoadMusicStream("resources/audio/background_theme.ogg");
    // if (sfxMap["glory_hit"].frameCount == 0) TraceLog(LOG_WARNING, "AUDIO: Failed to load glory_hit.wav");
    // if (musicMap["background_theme"].frameCount == 0) TraceLog(LOG_WARNING, "AUDIO: Failed to load background_theme.ogg");

    TraceLog(LOG_INFO, "AUDIO: Placeholder sounds loaded.");
}

// Unload all game sounds and music
void AudioManager::UnloadSounds() {
    for (auto const& [key, val] : sfxMap) {
        UnloadSound(val);
    }
    sfxMap.clear();

    for (auto const& [key, val] : musicMap) {
        UnloadMusicStream(val);
    }
    musicMap.clear();
    TraceLog(LOG_INFO, "AUDIO: All sounds and music unloaded.");
}

// Play background music
void AudioManager::PlayMusic(const std::string& musicName) {
    if (currentMusic.stream.buffer != nullptr) {
        StopMusicStream(currentMusic);
    }

    auto it = musicMap.find(musicName);
    if (it != musicMap.end()) {
        currentMusic = it->second;
        PlayMusicStream(currentMusic);
        SetMusicVolume(currentMusic, 0.5f); // Default music volume
        TraceLog(LOG_INFO, "AUDIO: Playing music: %s", musicName.c_str());
    } else {
        TraceLog(LOG_WARNING, "AUDIO: Music '%s' not found.", musicName.c_str());
    }
}

// Stop current background music
void AudioManager::StopMusic() {
    if (currentMusic.stream.buffer != nullptr) {
        StopMusicStream(currentMusic);
        TraceLog(LOG_INFO, "AUDIO: Stopped music.");
        currentMusic = { 0 };
    }
}

// Update streaming music (must be called every frame)
void AudioManager::UpdateMusicStream() {
    if (currentMusic.stream.buffer != nullptr && IsMusicStreamPlaying(currentMusic)) {
        ::UpdateMusicStream(currentMusic);
    }
}

// Play a sound effect with optional 3D spatialization
void AudioManager::PlaySFX(const std::string& sfxName, Vector3 position, float volume, float pitch) {
    auto it = sfxMap.find(sfxName);
    if (it != sfxMap.end()) {
        Sound sfx = it->second;

        // Basic 3D spatialization simulation (volume attenuation based on distance)
        float distance = Vector3Distance(listenerPosition, position);
        float maxDistance = 20.0f; // Max distance for sound to be audible
        float minVolume = 0.1f;    // Minimum audible volume
        float adjustedVolume = volume;

        if (distance < maxDistance) {
            adjustedVolume *= (1.0f - (distance / maxDistance));
            adjustedVolume = fmax(adjustedVolume, minVolume); // Ensure it's not completely silent too early
        } else {
            adjustedVolume = 0.0f; // Beyond max distance, not audible
        }
        
        SetSoundVolume(sfx, adjustedVolume);
        SetSoundPitch(sfx, pitch);
        PlaySound(sfx);
        // TraceLog(LOG_INFO, "AUDIO: Playing SFX: %s at volume %.2f", sfxName.c_str(), adjustedVolume);
    } else {
        // TraceLog(LOG_WARNING, "AUDIO: SFX '%s' not found.", sfxName.c_str());
    }
}

// Set the position of the audio listener (e.g., player camera position)
void AudioManager::SetListenerPosition(Vector3 position) {
    listenerPosition = position;
}
