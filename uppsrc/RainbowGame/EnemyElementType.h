#ifndef _RainbowGame_EnemyElementType_h_
#define _RainbowGame_EnemyElementType_h_

#include <Core/Core.h>

using namespace Upp;

/**
* Represents the elemental type of an enemy, used for world-specific mechanics
*/
enum class EnemyElementType {
    NORMAL,     // No special element
    WATER,      // Water element for Music Star
    LIGHTNING,  // Lightning element for Woods Star
    FIRE,       // Fire element
    EARTH,      // Earth element
    AIR         // Air element
};

#endif