#ifndef RAINBOWGAME_ASSETS_H
#define RAINBOWGAME_ASSETS_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>

using namespace Upp;

// Enum for sprite scaling
enum class SpriteScale {
    NORMAL,
    RETINA
};

class Assets {
public:
    Assets();
    ~Assets();
    
    void Load();
    void Dispose();
    
    // Getters
    BitmapFont* GetUIFont();
    // Note: TextureAtlas is not implemented in our Gdx implementation
    
    // Asset loading methods
    Music* GetMusic(const String& trackId);
    Sound* GetSound(const String& key);
    
private:
    // Internal asset cache
    ValueMap assetCache;
    
    // Pre-loaded assets
    // Note: TextureAtlas is not implemented in our Gdx implementation
    BitmapFont* uiFont;
    
    // Font generation (simplified)
    String GetMusicPath(const String& trackId);
    String GetSoundPath(const String& key);
    
    void* GetFromCache(const String& key);
    void PutInCache(const String& key, void* asset);
};

#endif