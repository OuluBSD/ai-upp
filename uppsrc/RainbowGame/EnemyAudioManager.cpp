#include "EnemyAudioManager.h"
#include <Core/Core.h>

EnemyAudioManager::EnemyAudioManager() : currentMusic(nullptr) {}

void EnemyAudioManager::LoadSound(const String& name, const String& path) {
    try {
        Sound* sound = new Sound();
        if (sound->LoadFromFile(path)) {
            soundEffects.GetAdd(name, One<Sound>(sound));
        } else {
            delete sound;
            LOG("EnemyAudioManager: Could not load sound: " + path);
        }
    } catch (...) {
        LOG("EnemyAudioManager: Could not load sound: " + path);
    }
}

void EnemyAudioManager::LoadMusic(const String& name, const String& path) {
    try {
        Music* music = new Music();
        if (music->LoadFromFile(path)) {
            musicTracks.GetAdd(name, One<Music>(music));
        } else {
            delete music;
            LOG("EnemyAudioManager: Could not load music: " + path);
        }
    } catch (...) {
        LOG("EnemyAudioManager: Could not load music: " + path);
    }
}

void EnemyAudioManager::PlaySound(const String& name) {
    Sound* sound = soundEffects.Get(name, nullptr);
    if (sound != nullptr) {
        sound->Play();
    }
}

void EnemyAudioManager::PlayMusic(const String& name, bool loop) {
    Music* newMusic = musicTracks.Get(name, nullptr);
    if (newMusic != nullptr) {
        // Stop current music if playing
        if (currentMusic != nullptr && currentMusic->IsPlaying()) {
            currentMusic->Stop();
        }
        
        // Play the music
        currentMusic = newMusic;
        currentMusic->SetLooping(loop);  // Assuming there's a SetLooping method
        currentMusic->Play();
    }
}

void EnemyAudioManager::StopMusic() {
    if (currentMusic != nullptr && currentMusic->IsPlaying()) {
        currentMusic->Stop();
    }
}

void EnemyAudioManager::Dispose() {
    // Dispose of all sound effects
    for (int i = 0; i < soundEffects.GetCount(); i++) {
        // Sound resources will be automatically disposed when One<> goes out of scope
    }
    soundEffects.Clear();
    
    // Dispose of all music tracks
    for (int i = 0; i < musicTracks.GetCount(); i++) {
        // Music resources will be automatically disposed when One<> goes out of scope
    }
    musicTracks.Clear();
    
    // Clear references
    currentMusic = nullptr;
}

bool EnemyAudioManager::IsMusicPlaying(const String& name) {
    Music* music = musicTracks.Get(name, nullptr);
    return music != nullptr && music->IsPlaying() && music == currentMusic;
}