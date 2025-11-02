#ifndef GDX_ASSETMANAGER_H
#define GDX_ASSETMANAGER_H

#include "Gdx.h"
#include "Texture.h"
#include "Sound.h"
#include "Music.h"
#include "BitmapFont.h"

using namespace Upp;

// Forward declarations
class AssetLoader;
class FileHandle;

class AssetManager {
public:
    AssetManager();
    ~AssetManager();
    
    void LoadTexture(const String& fileName);
    void LoadSound(const String& fileName);
    void LoadMusic(const String& fileName);
    void LoadFont(const String& fileName, int size);
    
    Texture* GetTexture(const String& fileName);
    Sound* GetSound(const String& fileName);
    Music* GetMusic(const String& fileName);
    BitmapFont* GetFont(const String& fileName);
    
    void Update(); // Process loading
    bool IsFinishedLoading();
    
    void Clear();
    
private:
    VectorMap<String, Texture*> textures;
    VectorMap<String, Sound*> sounds;
    VectorMap<String, Music*> music;
    VectorMap<String, BitmapFont*> fonts;
};

#endif