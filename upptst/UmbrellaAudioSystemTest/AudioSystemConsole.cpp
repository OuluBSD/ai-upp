#include "AudioSystemConsole.h"

// Implementation of AudioSystem
AudioSystem::AudioSystem() : initialized(false), masterVolume(1.0) {}

AudioSystem::~AudioSystem() {
    StopAllSounds();
}

bool AudioSystem::Initialize() {
    // In a real implementation, this would initialize the U++ sound system
    // For now, just mark as initialized
    initialized = true;
    LOG("AudioSystem initialized");
    return true;
}

bool AudioSystem::LoadSound(const String& id, const String& filePath, SoundType type) {
    if(!initialized) {
        LOG("AudioSystem not initialized");
        return false;
    }
    
    // Check if sound with this ID already exists
    for(int i = 0; i < soundIds.GetCount(); i++) {
        if(soundIds[i] == id) {
            LOG("Sound with ID '" + id + "' already exists");
            return false;
        }
    }
    
    // Add the new sound data
    soundIds.Add(id);
    soundPaths.Add(filePath);
    soundTypes.Add(type);
    soundVolumes.Add(1.0);
    soundLooping.Add(false);
    
    LOG("Loaded sound: " + id + " from " + filePath);
    return true;
}

bool AudioSystem::PlaySound(const String& id) {
    if(!initialized) {
        LOG("AudioSystem not initialized");
        return false;
    }
    
    int idx = GetSoundIndex(id);
    if(idx == -1) {
        LOG("Sound with ID '" + id + "' not found");
        return false;
    }
    
    if(soundTypes[idx] == SoundType::MUSIC) {
        LOG("Use PlayMusic() for music sounds");
        return false;
    }
    
    // In a real implementation, this would play the sound using U++ Sound system
    LOG("Playing sound: " + id);
    return true;
}

bool AudioSystem::PlayMusic(const String& id) {
    if(!initialized) {
        LOG("AudioSystem not initialized");
        return false;
    }
    
    int idx = GetSoundIndex(id);
    if(idx == -1) {
        LOG("Sound with ID '" + id + "' not found");
        return false;
    }
    
    if(soundTypes[idx] != SoundType::MUSIC) {
        LOG("Sound with ID '" + id + "' is not music type");
        return false;
    }
    
    // In a real implementation, this would play the music using U++ Sound system
    LOG("Playing music: " + id);
    return true;
}

void AudioSystem::StopAllSounds() {
    if(!initialized) return;
    
    LOG("Stopping all sounds");
    // In a real implementation, this would stop all playing sounds
}

void AudioSystem::StopMusic() {
    if(!initialized) return;
    
    LOG("Stopping music");
    // In a real implementation, this would stop all music
}

bool AudioSystem::SetVolume(const String& id, double volume) {
    if(!initialized) return false;
    
    int idx = GetSoundIndex(id);
    if(idx == -1) return false;
    
    soundVolumes[idx] = clamp(volume, 0.0, 1.0);
    LOG("Set volume for sound '" + id + "': " + AsString(soundVolumes[idx]));
    return true;
}

void AudioSystem::SetMasterVolume(double volume) {
    masterVolume = clamp(volume, 0.0, 1.0);
    LOG("Set master volume: " + AsString(masterVolume));
}

int AudioSystem::GetSoundIndex(const String& id) const {
    for(int i = 0; i < soundIds.GetCount(); i++) {
        if(soundIds[i] == id) {
            return i;
        }
    }
    return -1; // Not found
}

void AudioSystem::Update() {
    // In a real implementation, this would update ongoing audio operations
    // Check for finished sounds, manage streaming, etc.
}

// Global instance for easy access
AudioSystem& GetAudioSystem() {
    static AudioSystem audioSystem;
    if(!audioSystem.IsInitialized()) {
        audioSystem.Initialize();
    }
    return audioSystem;
}