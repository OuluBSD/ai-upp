#ifndef GDX_SDLWRAPPER_H
#define GDX_SDLWRAPPER_H

#include <Core/Core.h>

// Proper SDL2 includes using system approach
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>

using namespace Upp;

class SdlWrapper {
public:
    static bool Initialize();
    static void Shutdown();
    
    static SDL_Window* GetWindow() { return window; }
    static SDL_Renderer* GetRenderer() { return renderer; }
    
private:
    static SDL_Window* window;
    static SDL_Renderer* renderer;
    static bool initialized;
};

#endif
