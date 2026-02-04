#include "AudioSystemTest.h"

CONSOLE_APP_MAIN
{
    Cout() << "Testing AudioSystem functionality..." << EOL;

    // Get the global audio system instance
    AudioSystem& audio = GetAudioSystem();

    // Test initialization
    if(audio.IsInitialized()) {
        Cout() << "AudioSystem initialized successfully!" << EOL;
    } else {
        Cout() << "AudioSystem failed to initialize!" << EOL;
        return;
    }

    // Test loading sounds
    bool loadSFX = audio.LoadSound("jump_sound", "sounds/jump.wav", SoundType::SFX);
    bool loadMusic = audio.LoadSound("background_music", "music/theme.ogg", SoundType::MUSIC);
    bool loadVoice = audio.LoadSound("dialogue", "voice/dialogue.wav", SoundType::VOICE);

    Cout() << "SFX loaded: " << (loadSFX ? "YES" : "NO") << EOL;
    Cout() << "Music loaded: " << (loadMusic ? "YES" : "NO") << EOL;
    Cout() << "Voice loaded: " << (loadVoice ? "YES" : "NO") << EOL;

    // Test playing sounds
    bool playSFX = audio.PlaySound("jump_sound");
    bool playMusic = audio.PlayMusic("background_music");

    Cout() << "SFX played: " << (playSFX ? "YES" : "NO") << EOL;
    Cout() << "Music played: " << (playMusic ? "YES" : "NO") << EOL;

    // Test volume control
    bool volSet = audio.SetVolume("jump_sound", 0.75);
    Cout() << "Volume set for jump_sound: " << (volSet ? "YES" : "NO") << EOL;

    audio.SetMasterVolume(0.8);
    Cout() << "Master volume set to 0.8" << EOL;

    // Test sound lookup
    int soundIdx = audio.GetSoundIndex("background_music");
    Cout() << "Background music index: " << soundIdx << EOL;

    // Test stopping sounds
    audio.StopMusic();
    audio.StopAllSounds();
    Cout() << "Stopped music and all sounds" << EOL;

    Cout() << "AudioSystem test completed." << EOL;
}