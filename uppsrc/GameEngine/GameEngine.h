#ifndef UPP_GAMEENGINE_H
#define UPP_GAMEENGINE_H

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <CtrlCore/CtrlCore.h>
#include <Geometry/Geometry.h>
#include <GameLib/GameLib.h>

// Include relevant API packages that we'll need
#include <api/Screen/Screen.h>     // For windowing and input
#include <api/Graphics/Graphics.h> // For rendering

NAMESPACE_UPP_BEGIN

// Higher-level game engine features go here
#include "GameWindow.h"
#include "Game.h"
#include "AssetManager.h"
#include "Scene.h"

NAMESPACE_UPP_END

#endif