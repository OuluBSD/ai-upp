#include "Sound.h"

Sound::Sound() : chunk(nullptr), loaded(false) {}

Sound::Sound(const String& fileName) : chunk(nullptr), loaded(false) {
    LoadFromFile(fileName);
}

Sound::~Sound() {
    Unload();
}

bool Sound::LoadFromFile(const String& fileName) {
    Unload(); // Unload any existing sound
    
    chunk = Mix_LoadWAV(fileName);
    if (chunk == nullptr) {
        LOG("Unable to load sound " + fileName + ": " + Mix_GetError());
        return false;
    }
    
    loaded = true;
    return true;
}

void Sound::Unload() {
    if (chunk) {
        Mix_FreeChunk(chunk);
        chunk = nullptr;
    }
    loaded = false;
}

int Sound::Play(float volume) {
    if (!loaded) return -1;
    
    int channel = Mix_PlayChannel(-1, chunk, 0);
    if (channel != -1) {
        Mix_Volume(channel, (int)(volume * MIX_MAX_VOLUME));
    }
    return channel;
}

void Sound::Stop(int channel) {
    if (channel == -1) {
        Mix_HaltChannel(-1); // Stop all channels
    } else {
        Mix_HaltChannel(channel);
    }
}

void Sound::SetVolume(float volume) {
    if (loaded) {
        // This sets the volume for future plays, not currently playing instances
        Mix_VolumeChunk(chunk, (int)(volume * MIX_MAX_VOLUME));
    }
}