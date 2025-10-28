#pragma once
#ifndef _CtrlCore_MKeys_h_
#define _CtrlCore_MKeys_h_

namespace Upp {

enum CtrlCoreKeys {
	K_SHIFT = 0x0100,
	K_CTRL = 0x0200,
	K_ALT = 0x0400,
	K_CAPSLOCK = 0x0800,
	K_NUMLOCK = 0x1000,
	K_SCRLOCK = 0x2000,
	K_MOUSEMIDDLE = 0x4000,
	K_MOUSELEFT = 0x8000,
	K_MOUSERIGHT = 0x10000,
	K_MOUSEX1 = 0x20000,
	K_MOUSEX2 = 0x40000,
	K_REPEAT = 0x80000,
	K_MENU = 0x100000,
};

}

#endif