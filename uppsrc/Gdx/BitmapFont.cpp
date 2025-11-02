#include "BitmapFont.h"

BitmapFont::BitmapFont() : font(nullptr), loaded(false) {}

BitmapFont::BitmapFont(const String& fontFile, int size) : font(nullptr), loaded(false) {
    LoadFromFile(fontFile, size);
}

BitmapFont::~BitmapFont() {
    Unload();
}

bool BitmapFont::LoadFromFile(const String& fontFile, int size) {
    Unload(); // Unload any existing font
    
    font = TTF_OpenFont(fontFile, size);
    if (font == nullptr) {
        LOG("Unable to load font " + fontFile + ": " + TTF_GetError());
        return false;
    }
    
    loaded = true;
    return true;
}

void BitmapFont::Unload() {
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    loaded = false;
}

SDL_Texture* BitmapFont::RenderText(const String& text, SDL_Color color) {
    if (!loaded) return nullptr;
    
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, color);
    if (textSurface == nullptr) {
        LOG("Unable to render text surface: " + TTF_GetError());
        return nullptr;
    }
    
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(SdlWrapper::GetRenderer(), textSurface);
    SDL_FreeSurface(textSurface);
    
    return textTexture;
}