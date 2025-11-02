#ifndef RAINBOWGAME_INPUT_INPUTCONTROLLER_H
#define RAINBOWGAME_INPUT_INPUTCONTROLLER_H

#include <Core/Core.h>
#include <Gdx/Gdx.h>

using namespace Upp;

struct InputState {
    bool moveLeft = false;
    bool moveRight = false;
    bool jumpHeld = false;
    bool jumpPressed = false;
    bool attackPressed = false;
    bool glideHeld = false;
    bool pausePressed = false;
};

class InputController {
public:
    InputController();
    ~InputController();
    
    InputState Update();
    
private:
    InputState state;
};

#endif