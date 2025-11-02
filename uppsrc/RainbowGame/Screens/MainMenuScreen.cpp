#include "MainMenuScreen.h"

MainMenuScreen::MainMenuScreen(RainbowGame* game) : game(game) {
    // Initialize the main menu screen
}

MainMenuScreen::~MainMenuScreen() {
    // Clean up
}

void MainMenuScreen::Show() {
    // Called when this screen becomes active
    LOG("MainMenuScreen: Show");
}

void MainMenuScreen::Render(float deltaTime) {
    // Clear screen with a background color
    Graphics graphics;
    graphics.Clear(0.2f, 0.3f, 0.5f);  // Dark blue background
    
    // For now, just a simple render. In a real implementation, this would render UI elements
    LOG("MainMenuScreen: Render");
    
    // Check for input to transition to other screens
    Input::Update();
    if (Input::IsKeyJustPressed(Input::KeyCode::KEY_SPACE)) {
        // Transition to game screen when space is pressed
        // TODO: Implement GameScreen
        // game->SetScreen(new GameScreen(game));
    }
}

void MainMenuScreen::Hide() {
    // Called when this screen is replaced by another
    LOG("MainMenuScreen: Hide");
}