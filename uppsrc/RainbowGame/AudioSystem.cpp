#include "AudioSystem.h"

AudioSystem::AudioSystem(Assets* assets, Settings* settings) 
    : assets(assets), settings(settings), currentMusic(nullptr),
      fadeTimer(0.0f), fadeDuration(0.0f), targetVolume(1.0f) {
    // Initialize audio system
}

AudioSystem::~AudioSystem() {
    if (currentMusic) {
        currentMusic->Stop();
    }
}

void AudioSystem::Update(float delta) {
    if (currentMusic != nullptr && fadeDuration > 0.0f) {
        fadeTimer += delta;
        float alpha = min(1.0f, fadeTimer / fadeDuration);
        float volume = Lerp(0.0f, targetVolume, alpha);  // Linear interpolation
        currentMusic->SetVolume(volume * (IsMusicMuted() ? 0.0f : GetMusicVolume()));
        
        if (alpha >= 1.0f) {
            fadeDuration = 0.0f;
        }
    }
}

void AudioSystem::PlayMusic(const String& trackId, bool loop) {
    if (trackId.IsEmpty() || trackId == currentTrackId) {
        return;
    }
    
    StopMusic();
    currentTrackId = trackId;
    currentMusic = GetMusic(trackId);
    
    if (currentMusic) {
        // In our implementation, music looping is handled by the Music class
        targetVolume = GetMusicVolume();
        currentMusic->SetVolume(IsMusicMuted() ? 0.0f : targetVolume);
        currentMusic->Play();
    }
}

void AudioSystem::FadeTo(const String& trackId, float duration) {
    PlayMusic(trackId, true);
    fadeDuration = duration;
    fadeTimer = 0.0f;
    targetVolume = GetMusicVolume();
}

void AudioSystem::StopMusic() {
    if (currentMusic) {
        currentMusic->Stop();
        currentMusic = nullptr;
        currentTrackId.Clear();
    }
}

void AudioSystem::PlaySound(const String& key) {
    Sound* sound = GetSound(key);
    float volume = IsSfxMuted() ? 0.0f : GetSfxVolume();
    
    if (volume <= 0.0f) {
        return;
    }
    
    int id = sound->Play(volume);
    activeSounds.GetAdd(key) = (int64)id;  // Store the channel ID
}

void AudioSystem::StopSound(const String& key) {
    Sound* sound = GetSound(key);
    Value idValue = activeSounds.Get(key, Value((int64)0));
    int64 id = (int64)idValue;
    
    if (id != 0) {
        sound->Stop((int)id);
        activeSounds.Remove(key);
    }
}

float AudioSystem::GetMusicVolume() {
    return settings->GetMusicVolume();
}

bool AudioSystem::IsMusicMuted() {
    // In a real implementation, you might have a separate mute setting
    // For now, we'll assume music is muted if volume is 0
    return GetMusicVolume() <= 0.0f;
}

float AudioSystem::GetSfxVolume() {
    return settings->GetSoundVolume();
}

bool AudioSystem::IsSfxMuted() {
    // In a real implementation, you might have a separate mute setting
    // For now, we'll assume SFX is muted if volume is 0
    return GetSfxVolume() <= 0.0f;
}

Music* AudioSystem::GetMusic(const String& trackId) {
    return assets->GetMusic(trackId);
}

Sound* AudioSystem::GetSound(const String& key) {
    return assets->GetSound(key);
}