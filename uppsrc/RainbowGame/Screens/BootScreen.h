#ifndef RAINBOWGAME_SCREENS_BOOTSCREEN_H
#define RAINBOWGAME_SCREENS_BOOTSCREEN_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>

using namespace Upp;

class RainbowGame;

class BootScreen : public Screen {
public:
    BootScreen(RainbowGame* game);
    virtual ~BootScreen();
    
    virtual void Show() override;
    virtual void Render(float deltaTime) override;
    virtual void Hide() override;
    
private:
    RainbowGame* game;
};

#endif