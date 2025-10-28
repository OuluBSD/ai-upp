#pragma once
#ifndef _CtrlCore_DHCtrl_h_
#define _CtrlCore_DHCtrl_h_

#include "Ctrl.h"
#include <vector>

namespace Upp {

// DHCtrl (Direct Handle Control) - a control that has its own native window handle
// This is typically used for embedding external controls or using platform-specific features
class DHCtrl : public Ctrl {
public:
	virtual void State(int reason) override;
	
	// Platform-specific methods (in actual U++ these would have platform-specific implementations)
#ifdef PLATFORM_WIN32
	HWND GetHWND() const { return hwnd; }
	void SyncHWND();
	void OpenHWND();
	void CloseHWND();
	LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	bool PreprocessMessage(MSG& msg);
	static bool PreprocessMessageAll(MSG& msg);
	
protected:
	void NcCreate(HWND hwnd);
	void NcDestroy();
	void RemoveActive();
	
	HWND hwnd;
	Rect current_pos;
	bool current_visible;
	
	static std::vector<DHCtrl *> all_active;
	
	friend LRESULT CtrlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	friend void Ctrl::ClosePlatform();
	friend void Ctrl::OpenPlatform();
	
#elif defined(PLATFORM_POSIX) // For X11
	void* GetX11Window() const { return nullptr; } // Placeholder
	// Add X11 specific methods here
#endif

public:
	DHCtrl();
	virtual ~DHCtrl();
};

}

#endif