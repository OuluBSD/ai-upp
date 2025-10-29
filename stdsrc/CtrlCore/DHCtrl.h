#pragma once
#ifndef _CtrlCore_DHCtrl_h_
#define _CtrlCore_DHCtrl_h_

#include "CtrlCore.h"
#include "Ctrl.h"

namespace Upp {

#ifdef PLATFORM_WIN32

class DHCtrl : public Ctrl {
public:
	virtual void State(int reason) override;
	
	HWND GetHWND() const                    { return hwnd; }
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
	
	static Vector<DHCtrl *> all_active;
	
	friend LRESULT CtrlWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	friend void Ctrl::ClosePlatform();
	friend void Ctrl::OpenPlatform();
	
public:
	DHCtrl();
	virtual ~DHCtrl();
};

#endif

}

#endif