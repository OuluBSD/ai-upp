#ifndef RAINBOWGAME_SCREENS_MAINMENUSCREEN_H
#define RAINBOWGAME_SCREENS_MAINMENUSCREEN_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>

using namespace Upp;

class RainbowGame;

class MainMenuScreen : public Screen {
public:
    MainMenuScreen(RainbowGame* game);
    virtual ~MainMenuScreen();
    
    virtual void Show() override;
    virtual void Render(float deltaTime) override;
    virtual void Hide() override;
    
private:
    RainbowGame* game;
};

#endif