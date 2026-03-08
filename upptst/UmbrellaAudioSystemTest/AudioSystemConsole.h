#ifndef _AudioSystem_Console_h_
#define _AudioSystem_Console_h_

#include <Core/Core.h>

using namespace Upp;

// Sound type enum
enum class SoundType {
    SFX,
    MUSIC,
    VOICE
};

// Audio System Class - Console Implementation
class AudioSystem {
private:
    // Use simpler data structures to avoid guest class issues
    Vector<String> soundIds;
    Vector<String> soundPaths;
    Vector<SoundType> soundTypes;
    Vector<double> soundVolumes;
    Vector<bool> soundLooping;
    
    bool initialized;
    double masterVolume;
    
public:
    AudioSystem();
    ~AudioSystem();
    
    // Initialize the audio system
    bool Initialize();
    
    // Load a sound from file
    bool LoadSound(const String& id, const String& filePath, SoundType type);
    
    // Play a sound by ID
    bool PlaySound(const String& id);
    
    // Play music by ID
    bool PlayMusic(const String& id);
    
    // Stop all sounds
    void StopAllSounds();
    
    // Stop music
    void StopMusic();
    
    // Set volume for a specific sound
    bool SetVolume(const String& id, double volume);
    
    // Set master volume
    void SetMasterVolume(double volume);
    
    // Get sound index by ID
    int GetSoundIndex(const String& id) const;
    
    // Check if system is initialized
    bool IsInitialized() const { return initialized; }
    
    // Update - for ongoing audio management
    void Update();
};

// Global instance for easy access
AudioSystem& GetAudioSystem();

#endif