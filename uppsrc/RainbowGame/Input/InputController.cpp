#include "InputController.h"

InputController::InputController() {
    // Initialize input controller
}

InputController::~InputController() {
    // Cleanup
}

InputState InputController::Update() {
    // Update input state based on current key presses
    state.moveLeft = Input::IsKeyPressed(Input::KeyCode::KEY_A) || 
                     Input::IsKeyPressed(Input::KeyCode::KEY_LEFT);
    state.moveRight = Input::IsKeyPressed(Input::KeyCode::KEY_D) || 
                      Input::IsKeyPressed(Input::KeyCode::KEY_RIGHT);
    state.jumpHeld = Input::IsKeyPressed(Input::KeyCode::KEY_SPACE) ||
                     Input::IsKeyPressed(Input::KeyCode::KEY_W) ||
                     Input::IsKeyPressed(Input::KeyCode::KEY_UP);
    state.jumpPressed = Input::IsKeyJustPressed(Input::KeyCode::KEY_SPACE) ||
                        Input::IsKeyJustPressed(Input::KeyCode::KEY_W) ||
                        Input::IsKeyJustPressed(Input::KeyCode::KEY_UP);
    state.attackPressed = Input::IsKeyJustPressed(Input::KeyCode::KEY_RSHIFT) ||  // Right shift as attack
                          Input::IsKeyJustPressed(Input::KeyCode::KEY_X);          // Alternative: 'X' key
    state.glideHeld = Input::IsKeyPressed(Input::KeyCode::KEY_RSHIFT) ||          // Right shift as glide
                      Input::IsKeyPressed(Input::KeyCode::KEY_X);                  // Alternative: 'X' key
    state.pausePressed = Input::IsKeyJustPressed(Input::KeyCode::KEY_ESCAPE);
    
    return state;
}