#ifndef _ScriptIDE_TerminalWindow_h_
#define _ScriptIDE_TerminalWindow_h_

#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>

namespace Upp {

class TerminalWindow : public TopWindow {
public:
	typedef TerminalWindow CLASSNAME;
	TerminalWindow();

	void Execute(const String& cmdline, const String& dir);

private:
	TerminalCtrl terminal;
	PtyProcess pty;

	void Sync();
};

}

#endif
