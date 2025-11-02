#ifndef RAINBOWGAME_SCREENS_MAPEDITORSCREEN_H
#define RAINBOWGAME_SCREENS_MAPEDITORSCREEN_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>

using namespace Upp;

class RainbowGame;
class RainbowGameLaunchOptions;
class ModDefinition;

class MapEditorScreen : public Screen {
public:
    MapEditorScreen(RainbowGame* game, RainbowGameLaunchOptions* launchOptions, ModDefinition* modDefinition);
    virtual ~MapEditorScreen();
    
    virtual void Show() override;
    virtual void Render(float deltaTime) override;
    virtual void Hide() override;
    
private:
    RainbowGame* game;
    RainbowGameLaunchOptions* launchOptions;
    ModDefinition* modDefinition;
};

#endif