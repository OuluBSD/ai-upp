#include "Settings.h"

Settings::Settings() 
    : musicVolume(1.0f), soundVolume(1.0f), 
      fullscreen(false), resolutionWidth(800), resolutionHeight(600),
      languageCode("en") {
    // Initialize default settings
}

Settings::~Settings() {
    // Cleanup
}

void Settings::SetMusicVolume(float volume) {
    musicVolume = Clamp(volume, 0.0f, 1.0f);
}

float Settings::GetMusicVolume() const {
    return musicVolume;
}

void Settings::SetSoundVolume(float volume) {
    soundVolume = Clamp(volume, 0.0f, 1.0f);
}

float Settings::GetSoundVolume() const {
    return soundVolume;
}

void Settings::SetFullscreen(bool fullscreen) {
    this->fullscreen = fullscreen;
}

bool Settings::IsFullscreen() const {
    return fullscreen;
}

void Settings::SetResolution(int width, int height) {
    if (width > 0 && height > 0) {
        resolutionWidth = width;
        resolutionHeight = height;
    }
}

int Settings::GetResolutionWidth() const {
    return resolutionWidth;
}

int Settings::GetResolutionHeight() const {
    return resolutionHeight;
}

void Settings::SetLanguageCode(const String& code) {
    languageCode = code;
}

String Settings::GetLanguageCode() const {
    return languageCode;
}