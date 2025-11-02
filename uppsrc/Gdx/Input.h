#ifndef GDX_INPUT_H
#define GDX_INPUT_H

#include "Gdx.h"
#include "SdlWrapper.h"

using namespace Upp;

class Input {
public:
    enum class KeyCode {
        KEY_A = SDLK_a,
        KEY_B = SDLK_b,
        KEY_C = SDLK_c,
        KEY_D = SDLK_d,
        KEY_E = SDLK_e,
        KEY_F = SDLK_f,
        KEY_G = SDLK_g,
        KEY_H = SDLK_h,
        KEY_I = SDLK_i,
        KEY_J = SDLK_j,
        KEY_K = SDLK_k,
        KEY_L = SDLK_l,
        KEY_M = SDLK_m,
        KEY_N = SDLK_n,
        KEY_O = SDLK_o,
        KEY_P = SDLK_p,
        KEY_Q = SDLK_q,
        KEY_R = SDLK_r,
        KEY_S = SDLK_s,
        KEY_T = SDLK_t,
        KEY_U = SDLK_u,
        KEY_V = SDLK_v,
        KEY_W = SDLK_w,
        KEY_X = SDLK_x,
        KEY_Y = SDLK_y,
        KEY_Z = SDLK_z,
        KEY_0 = SDLK_0,
        KEY_1 = SDLK_1,
        KEY_2 = SDLK_2,
        KEY_3 = SDLK_3,
        KEY_4 = SDLK_4,
        KEY_5 = SDLK_5,
        KEY_6 = SDLK_6,
        KEY_7 = SDLK_7,
        KEY_8 = SDLK_8,
        KEY_9 = SDLK_9,
        KEY_F1 = SDLK_F1,
        KEY_F2 = SDLK_F2,
        KEY_F3 = SDLK_F3,
        KEY_F4 = SDLK_F4,
        KEY_F5 = SDLK_F5,
        KEY_F6 = SDLK_F6,
        KEY_F7 = SDLK_F7,
        KEY_F8 = SDLK_F8,
        KEY_F9 = SDLK_F9,
        KEY_F10 = SDLK_F10,
        KEY_F11 = SDLK_F11,
        KEY_F12 = SDLK_F12,
        KEY_SPACE = SDLK_SPACE,
        KEY_ENTER = SDLK_RETURN,
        KEY_ESCAPE = SDLK_ESCAPE,
        KEY_BACKSPACE = SDLK_BACKSPACE,
        KEY_TAB = SDLK_TAB,
        KEY_SHIFT_LEFT = SDLK_LSHIFT,
        KEY_SHIFT_RIGHT = SDLK_RSHIFT,
        KEY_CONTROL_LEFT = SDLK_LCTRL,
        KEY_CONTROL_RIGHT = SDLK_RCTRL,
        KEY_ALT_LEFT = SDLK_LALT,
        KEY_ALT_RIGHT = SDLK_RALT,
        KEY_UP = SDLK_UP,
        KEY_DOWN = SDLK_DOWN,
        KEY_LEFT = SDLK_LEFT,
        KEY_RIGHT = SDLK_RIGHT
    };
    
    Input();
    ~Input();
    
    static void Update();
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyJustPressed(KeyCode key);
    static bool IsKeyJustReleased(KeyCode key);
    
    static int GetMouseX();
    static int GetMouseY();
    static bool IsMousePressed(int button);
    static bool IsMouseJustPressed(int button);
    static bool IsMouseJustReleased(int button);
    
private:
    static Uint8 prevKeyState[SDL_NUM_SCANCODES];
    static Uint8 currKeyState[SDL_NUM_SCANCODES];
    static Uint32 prevMouseState;
    static Uint32 currMouseState;
    static int mouseX, mouseY;
};

#endif