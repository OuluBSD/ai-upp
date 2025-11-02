#include "AssetManager.h"

AssetManager::AssetManager() {
    // Initialize the maps
}

AssetManager::~AssetManager() {
    Clear();
}

void AssetManager::LoadTexture(const String& fileName) {
    Texture* texture = new Texture(fileName);
    textures.GetAdd(fileName) = texture;
}

void AssetManager::LoadSound(const String& fileName) {
    Sound* sound = new Sound(fileName);
    sounds.GetAdd(fileName) = sound;
}

void AssetManager::LoadMusic(const String& fileName) {
    Music* music = new Music(fileName);
    this->music.GetAdd(fileName) = music;
}

void AssetManager::LoadFont(const String& fileName, int size) {
    BitmapFont* font = new BitmapFont(fileName, size);
    fonts.GetAdd(fileName) = font;
}

Texture* AssetManager::GetTexture(const String& fileName) {
    int i = textures.Find(fileName);
    return (i >= 0) ? textures[i] : nullptr;
}

Sound* AssetManager::GetSound(const String& fileName) {
    int i = sounds.Find(fileName);
    return (i >= 0) ? sounds[i] : nullptr;
}

Music* AssetManager::GetMusic(const String& fileName) {
    int i = this->music.Find(fileName);
    return (i >= 0) ? this->music[i] : nullptr;
}

BitmapFont* AssetManager::GetFont(const String& fileName) {
    int i = fonts.Find(fileName);
    return (i >= 0) ? fonts[i] : nullptr;
}

void AssetManager::Update() {
    // In a real implementation, this would handle asynchronous loading
    // For now, it's just a placeholder
}

bool AssetManager::IsFinishedLoading() {
    // In a real implementation, this would check if all assets are loaded
    // For now, assume all assets are loaded immediately
    return true;
}

void AssetManager::Clear() {
    for (Texture* tex : textures) {
        delete tex;
    }
    textures.Clear();
    
    for (Sound* snd : sounds) {
        delete snd;
    }
    sounds.Clear();
    
    for (Music* mus : this->music) {
        delete mus;
    }
    this->music.Clear();
    
    for (BitmapFont* fnt : fonts) {
        delete fnt;
    }
    fonts.Clear();
}