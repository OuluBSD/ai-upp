#include "Texture.h"

Texture::Texture() : texture(nullptr), width(0), height(0), loaded(false) {}

Texture::Texture(const String& fileName) : texture(nullptr), width(0), height(0), loaded(false) {
    LoadFromFile(fileName);
}

Texture::~Texture() {
    Unload();
}

bool Texture::LoadFromFile(const String& fileName) {
    Unload(); // Unload any existing texture
    
    SDL_Surface* loadedSurface = IMG_Load(fileName);
    if (loadedSurface == nullptr) {
        LOG("Unable to load image " + fileName + ": " + IMG_GetError());
        return false;
    }
    
    // Create texture from surface
    texture = SDL_CreateTextureFromSurface(SdlWrapper::GetRenderer(), loadedSurface);
    if (texture == nullptr) {
        LOG("Unable to create texture from " + fileName + ": " + SDL_GetError());
        SDL_FreeSurface(loadedSurface);
        return false;
    }
    
    width = loadedSurface->w;
    height = loadedSurface->h;
    
    SDL_FreeSurface(loadedSurface);
    loaded = true;
    return true;
}

void Texture::Unload() {
    if (texture) {
        SDL_DestroyTexture(texture);
        texture = nullptr;
    }
    width = 0;
    height = 0;
    loaded = false;
}