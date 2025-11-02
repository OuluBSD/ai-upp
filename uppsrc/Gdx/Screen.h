#ifndef GDX_SCREEN_H
#define GDX_SCREEN_H

#include "Gdx.h"

using namespace Upp;

class Screen {
public:
    Screen() {}
    virtual ~Screen() {}
    
    virtual void Show() {}          // called when screen is set as current screen
    virtual void Render(float deltaTime) = 0;  // called every frame
    virtual void Resize(int width, int height) {}  // called when screen is resized
    virtual void Pause() {}         // called when application loses focus
    virtual void Resume() {}        // called when application gains focus
    virtual void Hide() {}          // called when screen is no longer current
};

#endif