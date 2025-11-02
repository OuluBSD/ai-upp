#ifndef GDX_GRAPHICS_H
#define GDX_GRAPHICS_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class Graphics {
public:
    Graphics();
    ~Graphics();
    
    void Clear(float r, float g, float b, float a = 1.0f);
    void Present();
    
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    
    static void SetClearColor(float r, float g, float b, float a = 1.0f);
    
private:
    int width, height;
    static float clearColorR, clearColorG, clearColorB, clearColorA;
};

#endif