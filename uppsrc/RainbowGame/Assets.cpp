#include "Assets.h"

Assets::Assets() : uiFont(nullptr) {
    // Initialize the assets
    Load();
}

Assets::~Assets() {
    Dispose();
}

void Assets::Load() {
    // Check if font file exists before attempting to load it
    String fontPath = "fonts/default.ttf";
    if (FileExists(fontPath)) {
        // Load font
        uiFont = new BitmapFont(fontPath, 16); // Default size 16
    } else {
        LOG("fonts/default.ttf not found, creating default font");
        // Create a simple default font
        uiFont = new BitmapFont(); // This will use system default
    }
}

void Assets::Dispose() {
    if (uiFont) {
        delete uiFont;
        uiFont = nullptr;
    }
    
    // Dispose of cached music and sounds
    for (int i = 0; i < assetCache.GetCount(); i++) {
        // In our implementation, we'll need to track what type of asset it is
        // This is simplified as compared to the pseudocode
    }
    
    assetCache.Clear();
}

BitmapFont* Assets::GetUIFont() {
    return uiFont;
}

Music* Assets::GetMusic(const String& trackId) {
    String path = GetMusicPath(trackId);
    Music* music = (Music*)GetFromCache(path);
    
    if (music == nullptr) {
        music = new Music(path);
        PutInCache(path, music);
    }
    
    return music;
}

Sound* Assets::GetSound(const String& key) {
    String path = GetSoundPath(key);
    Sound* sound = (Sound*)GetFromCache(path);
    
    if (sound == nullptr) {
        sound = new Sound(path);
        PutInCache(path, sound);
    }
    
    return sound;
}

String Assets::GetMusicPath(const String& trackId) {
    return "music/" + trackId + ".ogg";
}

String Assets::GetSoundPath(const String& key) {
    return "sounds/" + key + ".ogg";
}

void* Assets::GetFromCache(const String& key) {
    int i = assetCache.Find(key);
    if (i >= 0) {
        return (void*)assetCache.Get(key);
    }
    return nullptr;
}

void Assets::PutInCache(const String& key, void* asset) {
    assetCache.GetAdd(key) = (Value)asset;
}