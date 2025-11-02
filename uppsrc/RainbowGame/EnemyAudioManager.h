#ifndef _RainbowGame_EnemyAudioManager_h_
#define _RainbowGame_EnemyAudioManager_h_

#include <Core/Core.h>
#include <Gdx/Gdx.h>
#include "AudioSystem.h"  // Assuming this contains Sound and Music classes

using namespace Upp;

class EnemyAudioManager {
private:
    VectorMap<String, One<Sound>> soundEffects;
    VectorMap<String, One<Music>> musicTracks;
    Music* currentMusic;

public:
    EnemyAudioManager();
    
    // Load a sound effect by name
    void LoadSound(const String& name, const String& path);
    
    // Load a music track by name
    void LoadMusic(const String& name, const String& path);
    
    // Play a sound effect
    void PlaySound(const String& name);
    
    // Play a music track, optionally looping it
    void PlayMusic(const String& name, bool loop = false);
    
    // Stop currently playing music
    void StopMusic();
    
    // Get the currently playing music
    Music* GetCurrentMusic() const { return currentMusic; }
    
    // Dispose of all loaded audio resources
    void Dispose();
    
    // Check if the specified music is currently playing
    bool IsMusicPlaying(const String& name);
};

#endif