#include "Graphics.h"

float Graphics::clearColorR = 0.0f;
float Graphics::clearColorG = 0.0f;
float Graphics::clearColorB = 0.0f;
float Graphics::clearColorA = 1.0f;

Graphics::Graphics() : width(GDX_GRAPHICS_WIDTH), height(GDX_GRAPHICS_HEIGHT) {
    // Initialize with default values
}

Graphics::~Graphics() {
    // Cleanup handled by SDL wrapper
}

void Graphics::Clear(float r, float g, float b, float a) {
    SDL_SetRenderDrawColor(SdlWrapper::GetRenderer(), 
                          (Uint8)(r * 255), 
                          (Uint8)(g * 255), 
                          (Uint8)(b * 255), 
                          (Uint8)(a * 255));
    SDL_RenderClear(SdlWrapper::GetRenderer());
}

void Graphics::Present() {
    SDL_RenderPresent(SdlWrapper::GetRenderer());
}

void Graphics::SetClearColor(float r, float g, float b, float a) {
    clearColorR = r;
    clearColorG = g;
    clearColorB = b;
    clearColorA = a;
}