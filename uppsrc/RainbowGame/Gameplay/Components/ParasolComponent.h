#ifndef RAINBOWGAME_GAMEPLAY_COMPONENTS_PARASOLCOMPONENT_H
#define RAINBOWGAME_GAMEPLAY_COMPONENTS_PARASOLCOMPONENT_H

#include <Core/Core.h>
#include "Player.h"

using namespace Upp;

class ParasolComponent {
public:
    ParasolComponent();
    ~ParasolComponent();
    
    void Update(float delta, Player* player);
    void Render();
    
    void Open();
    void Close();
    bool IsOpen() const { return isOpen; }
    
private:
    bool isOpen;
    float openTimer;
    float openDuration;
};

#endif