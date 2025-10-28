#ifndef _CtrlCore_EventLoop_h_
#define _CtrlCore_EventLoop_h_

#include <CtrlCore/CtrlCore.h>

namespace Upp {

class EventLoop {
private:
	bool exit;
	int  exit_code;

public:
	EventLoop();
	~EventLoop();

	void Run();
	void Exit(int code = 0);
	
	bool IsRunning() const { return !exit; }
	int  GetExitCode() const { return exit_code; }
};

// Global event loop functions
void Run();
int  Run(int appmodal);
void ExitLoop(int code = 0);

// Modal event loop
int  Execute(Ctrl& ctrl);

// Event processing
void ProcessEvents();
bool ProcessEvent();

// GUI thread functions
void WakeupGuiThread();
bool IsMainThread();

}

#endif