#ifndef GDX_BITMAPFONT_H
#define GDX_BITMAPFONT_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class BitmapFont {
public:
    BitmapFont();
    BitmapFont(const String& fontFile, int size);
    ~BitmapFont();
    
    bool LoadFromFile(const String& fontFile, int size);
    void Unload();
    
    // For now, simple text rendering
    SDL_Texture* RenderText(const String& text, SDL_Color color);
    
private:
    TTF_Font* font;
    bool loaded;
};

#endif