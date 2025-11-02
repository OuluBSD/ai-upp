#ifndef RAINBOWGAME_AUDIOSYSTEM_H
#define RAINBOWGAME_AUDIOSYSTEM_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>
#include "Assets.h"
#include "Settings.h"

using namespace Upp;

class AudioSystem {
public:
    AudioSystem(Assets* assets, Settings* settings);
    ~AudioSystem();
    
    void Update(float delta);
    
    void PlayMusic(const String& trackId, bool loop = true);
    void FadeTo(const String& trackId, float duration);
    void StopMusic();
    
    void PlaySound(const String& key);
    void StopSound(const String& key);
    
    // Settings integration
    float GetMusicVolume();
    bool IsMusicMuted();
    float GetSfxVolume();
    bool IsSfxMuted();
    
private:
    Assets* assets;
    Settings* settings;
    
    ValueMap activeSounds;
    Music* currentMusic;
    String currentTrackId;
    
    float fadeTimer;
    float fadeDuration;
    float targetVolume;
    
    // Helper methods
    Music* GetMusic(const String& trackId);
    Sound* GetSound(const String& key);
};

#endif