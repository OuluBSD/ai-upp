#ifndef GDX_TEXTURE_H
#define GDX_TEXTURE_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class Texture {
public:
    Texture();
    Texture(const String& fileName);
    ~Texture();
    
    bool LoadFromFile(const String& fileName);
    void Unload();
    
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    SDL_Texture* GetSdlTexture() const { return texture; }
    
private:
    SDL_Texture* texture;
    int width, height;
    bool loaded;
};

#endif