#include <Core/Core.h>
#include <GameEngine/GameEngine.h>

using namespace Upp;

// Simple example game that extends the base Game class
class MyGame : public Game {
public:
	MyGame() {
		// Set up the main window
		GetMainWindow().Title("My Game Example");
		GetMainWindow().Sizeable(true);
		GetMainWindow().NoCenter();
		GetMainWindow().SetRect(100, 100, 800, 600);
	}

	virtual void Initialize() override {
		Game::Initialize();
		LOG("Game initialized");
	}

	virtual void LoadContent() override {
		Game::LoadContent();
		LOG("Game content loaded");
	}

	virtual void Update(double deltaTime) override {
		// Update game state here
		// deltaTime is the time in seconds since the last update
		Game::Update(deltaTime);
	}

	virtual void Render(Draw& draw) override {
		// Render game content here
		// Clear with a dark blue background
		draw.DrawRect(GetMainWindow().GetSize(), Color(20, 20, 60));
		
		// Draw some simple content
		Size sz = GetMainWindow().GetSize();
		draw.DrawText(20, 20, "Hello Game Engine!", Arial(20), White());
		draw.DrawRect(100, 100, 200, 200, Color(100, 150, 200));
		
		Game::Render(draw);
	}

	virtual void UnloadContent() override {
		LOG("Game content unloaded");
		Game::UnloadContent();
	}
};

GUI_APP_MAIN {
	MyGame game;
	game.Run();
}