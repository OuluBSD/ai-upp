#ifndef GDX_MUSIC_H
#define GDX_MUSIC_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class Music {
public:
    Music();
    Music(const String& fileName);
    ~Music();
    
    bool LoadFromFile(const String& fileName);
    void Unload();
    
    void Play(float volume = 1.0f);
    void Pause();
    void Resume();
    void Stop();
    bool IsPlaying();
    
    void SetVolume(float volume);
    
private:
    String fileName;
    bool loaded;
    bool playing;
};

#endif