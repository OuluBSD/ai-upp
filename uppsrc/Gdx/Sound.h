#ifndef GDX_SOUND_H
#define GDX_SOUND_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class Sound {
public:
    Sound();
    Sound(const String& fileName);
    ~Sound();
    
    bool LoadFromFile(const String& fileName);
    void Unload();
    
    int Play(float volume = 1.0f);
    void Stop(int channel = -1);
    void SetVolume(float volume);
    
private:
    Mix_Chunk* chunk;
    bool loaded;
};

#endif