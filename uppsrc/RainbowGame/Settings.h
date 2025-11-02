#ifndef RAINBOWGAME_SETTINGS_H
#define RAINBOWGAME_SETTINGS_H

#include <Core/Core.h>

using namespace Upp;

class Settings {
public:
    Settings();
    ~Settings();
    
    // Audio settings
    void SetMusicVolume(float volume);
    float GetMusicVolume() const;
    
    void SetSoundVolume(float volume);
    float GetSoundVolume() const;
    
    // Graphics settings
    void SetFullscreen(bool fullscreen);
    bool IsFullscreen() const;
    
    void SetResolution(int width, int height);
    int GetResolutionWidth() const;
    int GetResolutionHeight() const;
    
    // Language settings
    void SetLanguageCode(const String& code);
    String GetLanguageCode() const;
    
private:
    float musicVolume;
    float soundVolume;
    bool fullscreen;
    int resolutionWidth;
    int resolutionHeight;
    String languageCode;
};

#endif