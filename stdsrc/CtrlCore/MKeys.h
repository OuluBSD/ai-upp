#pragma once
#ifndef _CtrlCore_MKeys_h_
#define _CtrlCore_MKeys_h_

namespace Upp {

// Keyboard key codes
enum CtrlCoreKeys {
	// Modifier keys
	K_SHIFT = 0x0100,
	K_CTRL = 0x0200,
	K_ALT = 0x0400,
	K_CAPSLOCK = 0x0800,
	K_NUMLOCK = 0x1000,
	K_SCRLOCK = 0x2000,
	K_REPEAT = 0x80000,
	K_MENU = 0x100000,
	
	// Mouse buttons (included in key codes for historical reasons)
	K_MOUSEMIDDLE = 0x4000,
	K_MOUSELEFT = 0x8000,
	K_MOUSERIGHT = 0x10000,
	K_MOUSEX1 = 0x20000,
	K_MOUSEX2 = 0x40000,
	
	// Standard ASCII keys
	K_BACKSPACE = 0x08,
	K_TAB = 0x09,
	K_RETURN = 0x0D,
	K_ENTER = 0x0D,
	K_ESCAPE = 0x1B,
	K_SPACE = 0x20,
	K_EXCLAM = 0x21,
	K_QUOTE = 0x22,
	K_NUMBER = 0x23,
	K_DOLLAR = 0x24,
	K_PERCENT = 0x25,
	K_AMPERSAND = 0x26,
	K_APOSTROPHE = 0x27,
	K_LEFTPAREN = 0x28,
	K_RIGHTPAREN = 0x29,
	K_ASTERISK = 0x2A,
	K_PLUS = 0x2B,
	K_COMMA = 0x2C,
	K_MINUS = 0x2D,
	K_PERIOD = 0x2E,
	K_SLASH = 0x2F,
	K_0 = 0x30,
	K_1 = 0x31,
	K_2 = 0x32,
	K_3 = 0x33,
	K_4 = 0x34,
	K_5 = 0x35,
	K_6 = 0x36,
	K_7 = 0x37,
	K_8 = 0x38,
	K_9 = 0x39,
	K_COLON = 0x3A,
	K_SEMICOLON = 0x3B,
	K_LESS = 0x3C,
	K_EQUAL = 0x3D,
	K_GREATER = 0x3E,
	K_QUESTION = 0x3F,
	K_AT = 0x40,
	K_A = 0x41,
	K_B = 0x42,
	K_C = 0x43,
	K_D = 0x44,
	K_E = 0x45,
	K_F = 0x46,
	K_G = 0x47,
	K_H = 0x48,
	K_I = 0x49,
	K_J = 0x4A,
	K_K = 0x4B,
	K_L = 0x4C,
	K_M = 0x4D,
	K_N = 0x4E,
	K_O = 0x4F,
	K_P = 0x50,
	K_Q = 0x51,
	K_R = 0x52,
	K_S = 0x53,
	K_T = 0x54,
	K_U = 0x55,
	K_V = 0x56,
	K_W = 0x57,
	K_X = 0x58,
	K_Y = 0x59,
	K_Z = 0x5A,
	K_LEFTBRACKET = 0x5B,
	K_BACKSLASH = 0x5C,
	K_RIGHTBRACKET = 0x5D,
	K_CARET = 0x5E,
	K_UNDERSCORE = 0x5F,
	K_BACKQUOTE = 0x60,
	K_LEFTBRACE = 0x7B,
	K_PIPE = 0x7C,
	K_RIGHTBRACE = 0x7D,
	K_TILDE = 0x7E,
	K_DELETE = 0x7F,
	
	// Numeric keypad keys
	K_NUMPAD0 = 0x6000,
	K_NUMPAD1 = 0x6001,
	K_NUMPAD2 = 0x6002,
	K_NUMPAD3 = 0x6003,
	K_NUMPAD4 = 0x6004,
	K_NUMPAD5 = 0x6005,
	K_NUMPAD6 = 0x6006,
	K_NUMPAD7 = 0x6007,
	K_NUMPAD8 = 0x6008,
	K_NUMPAD9 = 0x6009,
	K_MULTIPLY = 0x600A,
	K_ADD = 0x600B,
	K_SEPARATOR = 0x600C,
	K_SUBTRACT = 0x600D,
	K_DECIMAL = 0x600E,
	K_DIVIDE = 0x600F,
	
	// Function keys
	K_F1 = 0x7000,
	K_F2 = 0x7001,
	K_F3 = 0x7002,
	K_F4 = 0x7003,
	K_F5 = 0x7004,
	K_F6 = 0x7005,
	K_F7 = 0x7006,
	K_F8 = 0x7007,
	K_F9 = 0x7008,
	K_F10 = 0x7009,
	K_F11 = 0x700A,
	K_F12 = 0x700B,
	K_F13 = 0x700C,
	K_F14 = 0x700D,
	K_F15 = 0x700E,
	K_F16 = 0x700F,
	K_F17 = 0x7010,
	K_F18 = 0x7011,
	K_F19 = 0x7012,
	K_F20 = 0x7013,
	K_F21 = 0x7014,
	K_F22 = 0x7015,
	K_F23 = 0x7016,
	K_F24 = 0x7017,
	
	// Navigation keys
	K_LEFT = 0x7020,
	K_UP = 0x7021,
	K_RIGHT = 0x7022,
	K_DOWN = 0x7023,
	K_HOME = 0x7024,
	K_END = 0x7025,
	K_PGUP = 0x7026,
	K_PGDN = 0x7027,
	
	// Editing keys
	K_INSERT = 0x7030,
	K_BACKSPACE = 0x08,
	K_DELETE = 0x7F,
	K_TAB = 0x09,
	K_RETURN = 0x0D,
	K_ENTER = 0x0D,
	K_ESCAPE = 0x1B,
	K_SPACE = 0x20,
	
	// Numeric keypad keys
	K_NUMPAD0 = 0x7040,
	K_NUMPAD1 = 0x7041,
	K_NUMPAD2 = 0x7042,
	K_NUMPAD3 = 0x7043,
	K_NUMPAD4 = 0x7044,
	K_NUMPAD5 = 0x7045,
	K_NUMPAD6 = 0x7046,
	K_NUMPAD7 = 0x7047,
	K_NUMPAD8 = 0x7048,
	K_NUMPAD9 = 0x7049,
	K_MULTIPLY = 0x704A,
	K_ADD = 0x704B,
	K_SEPARATOR = 0x704C,
	K_SUBTRACT = 0x704D,
	K_DECIMAL = 0x704E,
	K_DIVIDE = 0x704F,
	
	// Special keys
	K_CAPITAL = 0x7050,
	K_NUMLOCK = 0x1000,
	K_SCROLL = 0x2000,
	K_LSHIFT = 0x7051,
	K_RSHIFT = 0x7052,
	K_LCONTROL = 0x7053,
	K_RCONTROL = 0x7054,
	K_LMENU = 0x7055,
	K_RMENU = 0x7056,
	
	// System keys
	K_LWIN = 0x7060,
	K_RWIN = 0x7061,
	K_APPS = 0x7062,
	K_SLEEP = 0x7063,
	K_POWER = 0x7064,
	
	// Media keys
	K_VOLUME_MUTE = 0x7070,
	K_VOLUME_DOWN = 0x7071,
	K_VOLUME_UP = 0x7072,
	K_MEDIA_NEXT_TRACK = 0x7073,
	K_MEDIA_PREV_TRACK = 0x7074,
	K_MEDIA_STOP = 0x7075,
	K_MEDIA_PLAY_PAUSE = 0x7076,
	K_BROWSER_BACK = 0x7077,
	K_BROWSER_FORWARD = 0x7078,
	K_BROWSER_REFRESH = 0x7079,
	K_BROWSER_STOP = 0x707A,
	K_BROWSER_SEARCH = 0x707B,
	K_BROWSER_FAVORITES = 0x707C,
	K_BROWSER_HOME = 0x707D,
	
	// Browser keys
	K_BROWSER_BACK = 0x7077,
	K_BROWSER_FORWARD = 0x7078,
	K_BROWSER_REFRESH = 0x7079,
	K_BROWSER_STOP = 0x707A,
	K_BROWSER_SEARCH = 0x707B,
	K_BROWSER_FAVORITES = 0x707C,
	K_BROWSER_HOME = 0x707D,
	
	// Launcher keys
	K_LAUNCH_MAIL = 0x7080,
	K_LAUNCH_MEDIA_SELECT = 0x7081,
	K_LAUNCH_APP1 = 0x7082,
	K_LAUNCH_APP2 = 0x7083,
	
	// OEM keys (keyboard-specific)
	K_OEM_1 = 0x7090,      // Usually ';:' for US
	K_OEM_PLUS = 0x7091,   // '+' any country
	K_OEM_COMMA = 0x7092,  // ',' any country
	K_OEM_MINUS = 0x7093,  // '-' any country
	K_OEM_PERIOD = 0x7094,// '.' any country
	K_OEM_2 = 0x7095,      // Usually '/?' for US
	K_OEM_3 = 0x7096,      // Usually '`~' for US
	K_OEM_4 = 0x7097,      // Usually '[{' for US
	K_OEM_5 = 0x7098,      // Usually '\|' for US
	K_OEM_6 = 0x7099,      // Usually ']}' for US
	K_OEM_7 = 0x709A,      // Usually ''"' for US
	K_OEM_8 = 0x709B,
	
	// Delta value for key classification
	K_DELTA = 0x200000,
	
	// Character limit for Unicode processing
	K_CHAR_LIM = 0x200000,
	
	// Key up flag (OR'd with key code)
	K_KEYUP = 0x4000000,
	
	// System-specific keys
	K_ALTGR = 0x8000000,    // AltGr key (European keyboards)
	K_WIN = 0x10000000,     // Windows key
	K_CONTEXT = 0x20000000, // Context menu key
	
	// Common key combinations
	K_CTRL_A = K_CTRL | K_A,
	K_CTRL_C = K_CTRL | K_C,
	K_CTRL_V = K_CTRL | K_V,
	K_CTRL_X = K_CTRL | K_X,
	K_CTRL_Z = K_CTRL | K_Z,
	K_CTRL_Y = K_CTRL | K_Y,
	K_ALT_F4 = K_ALT | K_F4,
	K_CTRL_F4 = K_CTRL | K_F4,
	K_SHIFT_F10 = K_SHIFT | K_F10,
	K_CTRL_INS = K_CTRL | K_INSERT,
	K_SHIFT_INS = K_SHIFT | K_INSERT,
	K_SHIFT_DEL = K_SHIFT | K_DELETE,
	K_CTRL_DEL = K_CTRL | K_DELETE,
};

// Flags for key combinations
enum CtrlCoreFlags {
	KF_SHIFT = K_SHIFT,
	KF_CTRL = K_CTRL,
	KF_ALT = K_ALT,
	KF_REPEAT = K_REPEAT,
	KF_MENU = K_MENU,
};

}

#endif