#include "BootScreen.h"
#include "MainMenuScreen.h"  // We'll create this later

BootScreen::BootScreen(RainbowGame* game) : game(game) {
    // Initialize the boot screen
}

BootScreen::~BootScreen() {
    // Clean up
}

void BootScreen::Show() {
    // Called when this screen becomes active
    LOG("BootScreen: Show");
}

void BootScreen::Render(float deltaTime) {
    // For now, immediately transition to main menu after a short time
    // In a real implementation, this might show a splash screen or loading progress
    static float timer = 0;
    timer += deltaTime;
    
    if (timer > 2.0f) {  // After 2 seconds, go to main menu
        game->SetScreen(new MainMenuScreen(game));
        timer = 0;
    }
}

void BootScreen::Hide() {
    // Called when this screen is replaced by another
    LOG("BootScreen: Hide");
}