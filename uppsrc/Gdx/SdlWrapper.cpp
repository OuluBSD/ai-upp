#include "SdlWrapper.h"
#include <Core/Core.h>

SDL_Window* SdlWrapper::window = nullptr;
SDL_Renderer* SdlWrapper::renderer = nullptr;
bool SdlWrapper::initialized = false;

bool SdlWrapper::Initialize() {
    if (initialized) return true;
    
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        LOG("SDL Error: " << SDL_GetError());
        return false;
    }
    
    // Initialize SDL_image for image loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        LOG("SDL_image Error: " << IMG_GetError());
        return false;
    }
    
    // Initialize SDL_ttf for font rendering
    if (TTF_Init() < 0) {
        LOG("SDL_ttf Error: " << TTF_GetError());
        return false;
    }
    
    // Initialize SDL_mixer for audio
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        LOG("SDL_mixer Error: " << Mix_GetError());
        return false;
    }
    
    // Create window
    window = SDL_CreateWindow(
        "Rainbow Game",
        SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED,
        800,
        600,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        LOG("Window creation error: " << SDL_GetError());
        return false;
    }
    
    // Create renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        LOG("Renderer creation error: " << SDL_GetError());
        return false;
    }
    
    initialized = true;
    return true;
}

void SdlWrapper::Shutdown() {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    
    initialized = false;
}
