#ifndef GDX_APPLICATION_H
#define GDX_APPLICATION_H

#include "Gdx.h"
#include <Core/Core.h>

using namespace Upp;

class Application {
public:
    Application() {}
    virtual ~Application() {}
    
    virtual void Create() = 0;
    virtual void Render() = 0;
    virtual void Dispose() = 0;
    virtual void Resize(int width, int height) {}
    virtual void Pause() {}
    virtual void Resume() {}
    
    void SetLogLevel(int level) { logLevel = level; }
    void Log(const String& tag, const String& message) { 
        if (logLevel >= 0) {
            LOG(tag + ": " + message);
        }
    }
    
private:
    int logLevel = 0;
};

class Game : public Application {
public:
    Game() {}
    virtual ~Game() { if (currentScreen) delete currentScreen; }
    
    void SetScreen(Screen* screen) {
        if (currentScreen) {
            currentScreen->Hide();
            delete currentScreen;
        }
        currentScreen = screen;
        if (currentScreen) {
            currentScreen->Show();
            currentScreen->Resize(GDX_GRAPHICS_WIDTH, GDX_GRAPHICS_HEIGHT);
        }
    }
    
    Screen* GetScreen() { return currentScreen; }
    
    virtual void Create() override {
        currentScreen = nullptr;
    }
    
    virtual void Render() override {
        if (currentScreen) {
            currentScreen->Render(0);  // delta time parameter
        }
    }
    
    virtual void Dispose() override {
        if (currentScreen) {
            currentScreen->Hide();
            delete currentScreen;
            currentScreen = nullptr;
        }
    }

private:
    Screen* currentScreen = nullptr;
};

#endif