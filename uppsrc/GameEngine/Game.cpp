#include "GameEngine.h"
#include <memory>

NAMESPACE_UPP

Game::Game() : renderer(std::make_unique<Renderer>()) {
	// Set up the main window callbacks
	main_window.SetRenderCallback([this](Draw& w) {
		// Initialize renderer if not done already
		if (!renderer_initialized) {
			renderer->Initialize(main_window);
			renderer_initialized = true;
		}
		Render(w);
	});

	main_window.SetUpdateCallback([this](double dt) {
		delta_time = dt;
		Update(dt);
	});

	// Set up the game loop callback
	main_window.SetGameLoopCallback([this]() {
		// Any additional game loop logic can go here
	});
}

Game::~Game() {
	UnloadContent();
}

void Game::Initialize() {
	// Initialize game systems here
}

void Game::LoadContent() {
	// Load game assets here
}

void Game::UnloadContent() {
	// Unload game assets here
}

void Game::Update(double deltaTime) {
	// Update game logic here
	// This is called from the game thread with the delta time
}

void Game::Render(Draw& draw) {
	// Render game content here using our renderer
	// This is called from the UI thread during Paint
	if (renderer) {
		renderer->Render(draw);
	} else {
		// Fallback rendering if renderer not initialized
		draw.DrawRect(main_window.GetSize(), Color(20, 20, 60)); // Dark blue background
	}
}

void Game::Run() {
	running = true;

	// Initialize the game
	Initialize();
	LoadContent();

	// Start the game loop in our GameWindow
	main_window.StartGameLoop();

	// Run the main window modal
	main_window.Run();

	// When the window closes, stop the game loop
	main_window.StopGameLoop();
	running = false;

	// Clean up
	UnloadContent();
}

void Game::Exit() {
	running = false;
	main_window.Break(); // Close the window
}

END_UPP_NAMESPACE
