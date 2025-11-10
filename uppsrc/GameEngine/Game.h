#ifndef UPP_GAME_H
#define UPP_GAME_H

#include <Core/Core.h>
#include <Draw/Draw.h>
#include <CtrlCore/CtrlCore.h>
#include <Geometry/Geometry.h>
#include <GameEngine/GameEngine.h>
#include <GameEngine/GameWindow.h>

// Define missing namespace macros if not already defined  
#ifndef NAMESPACE_UPP_BEGIN
#define NAMESPACE_UPP_BEGIN namespace Upp {
#define NAMESPACE_UPP_END }
#endif

// Note: This header should be included from within the namespace context
// as it depends on classes defined in GameWindow.h which are in the Upp namespace

class Game {
public:
	Game();
	virtual ~Game();

	// Game lifecycle methods
	virtual void Initialize();
	virtual void LoadContent();
	virtual void UnloadContent();
	virtual void Update(double deltaTime);
	virtual void Render(Draw& draw);
	
	// Game management
	void Run();
	void Exit();
	
	// Accessors
	GameWindow& GetMainWindow() { return main_window; }
	const GameWindow& GetMainWindow() const { return main_window; }
	
	// Game timing
	double GetDeltaTime() const { return delta_time; }
	
protected:
	GameWindow main_window;
	bool running = false;
	double delta_time = 0.0;
	
private:
	void GameLoop();
};

#endif