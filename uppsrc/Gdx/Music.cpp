#include "Music.h"

Music::Music() : loaded(false), playing(false) {}

Music::Music(const String& fileName) : fileName(fileName), loaded(false), playing(false) {
    LoadFromFile(fileName);
}

Music::~Music() {
    Unload();
}

bool Music::LoadFromFile(const String& file) {
    Unload(); // Unload any existing music
    
    if (Mix_LoadMUS(file) == nullptr) {
        LOG("Unable to load music " + file + ": " + Mix_GetError());
        return false;
    }
    
    fileName = file;
    loaded = true;
    return true;
}

void Music::Unload() {
    if (loaded) {
        if (playing) {
            Stop();
        }
        // In SDL_mixer, we don't free the music here as Mix_FreeMusic is not needed
        // for files loaded with Mix_LoadMUS
        loaded = false;
    }
}

void Music::Play(float volume) {
    if (!loaded) return;
    
    Mix_Music* music = Mix_LoadMUS(fileName);
    if (music == nullptr) {
        LOG(String("Failed to load music for playback: ") + String(Mix_GetError()));
        return;
    }
    
    Mix_PlayMusic(music, -1); // Loop indefinitely
    SetVolume(volume);
    playing = true;
}

void Music::Pause() {
    if (playing) {
        Mix_PauseMusic();
        playing = false;
    }
}

void Music::Resume() {
    if (!playing) {
        Mix_ResumeMusic();
        playing = true;
    }
}

void Music::Stop() {
    Mix_HaltMusic();
    playing = false;
}

bool Music::IsPlaying() {
    return Mix_PlayingMusic() && playing;
}

void Music::SetVolume(float volume) {
    Mix_VolumeMusic((int)(volume * MIX_MAX_VOLUME));
}