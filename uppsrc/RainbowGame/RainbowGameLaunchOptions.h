#ifndef RAINBOWGAME_RAINBOWGAMELAUNCHOPTIONS_H
#define RAINBOWGAME_RAINBOWGAMELAUNCHOPTIONS_H

#include <Core/Core.h>

using namespace Upp;

class RainbowGameLaunchOptions {
public:
    RainbowGameLaunchOptions() : editorMode(false), fromConstructor(false) {}
    RainbowGameLaunchOptions(bool editorMode) : editorMode(editorMode), fromConstructor(true) {}
    
    bool IsEditorMode() const { return editorMode; }
    bool IsFromConstructor() const { return fromConstructor; }
    
private:
    bool editorMode;
    bool fromConstructor;
};

#endif