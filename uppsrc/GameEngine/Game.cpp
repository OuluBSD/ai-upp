#include "Game.h"
#include <Core/Core.h>
#include <Draw/Draw.h>
#include <thread>
#include <chrono>

NAMESPACE_UPP_BEGIN

Game::Game() {
	// Set up the main window callbacks
	main_window.SetRenderCallback([this](Draw& w) { 
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
	// Render game content here
	// This is called from the UI thread during Paint
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

NAMESPACE_UPP_END