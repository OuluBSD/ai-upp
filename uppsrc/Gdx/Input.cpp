#include "Input.h"

Uint8 Input::prevKeyState[SDL_NUM_SCANCODES] = {0};
Uint8 Input::currKeyState[SDL_NUM_SCANCODES] = {0};
Uint32 Input::prevMouseState = 0;
Uint32 Input::currMouseState = 0;
int Input::mouseX = 0;
int Input::mouseY = 0;

Input::Input() {
    // Initialize input state
    SDL_GetKeyboardState(NULL); // Just to ensure we can access the keyboard state
}

Input::~Input() {
    // Cleanup
}

void Input::Update() {
    // Copy current state to previous state
    memcpy(prevKeyState, currKeyState, SDL_NUM_SCANCODES);
    prevMouseState = currMouseState;
    
    // Get current keyboard state
    const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
    memcpy(currKeyState, keyboardState, SDL_NUM_SCANCODES);
    
    // Get current mouse state
    currMouseState = SDL_GetMouseState(&mouseX, &mouseY);
}

bool Input::IsKeyPressed(KeyCode key) {
    SDL_Scancode scanCode = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key));
    return currKeyState[scanCode] != 0;
}

bool Input::IsKeyJustPressed(KeyCode key) {
    SDL_Scancode scanCode = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key));
    return (currKeyState[scanCode] != 0) && (prevKeyState[scanCode] == 0);
}

bool Input::IsKeyJustReleased(KeyCode key) {
    SDL_Scancode scanCode = SDL_GetScancodeFromKey(static_cast<SDL_Keycode>(key));
    return (currKeyState[scanCode] == 0) && (prevKeyState[scanCode] != 0);
}

int Input::GetMouseX() {
    return mouseX;
}

int Input::GetMouseY() {
    return mouseY;
}

bool Input::IsMousePressed(int button) {
    return (currMouseState & SDL_BUTTON(button)) != 0;
}

bool Input::IsMouseJustPressed(int button) {
    return ((currMouseState & SDL_BUTTON(button)) != 0) && 
           ((prevMouseState & SDL_BUTTON(button)) == 0);
}

bool Input::IsMouseJustReleased(int button) {
    return ((currMouseState & SDL_BUTTON(button)) == 0) && 
           ((prevMouseState & SDL_BUTTON(button)) != 0);
}