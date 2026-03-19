#ifndef _Maestro_TerminalBuffer_h_
#define _Maestro_TerminalBuffer_h_

#include <Terminal/Terminal.h>

namespace Upp {

class TerminalBuffer {
public:
	TerminalBuffer();

	void SetSize(int cols, int rows);
	void Write(const String& data);
	
	// Structured access
	int GetLineCount() const;
	String GetLine(int line) const;
	String GetVisibleScreen() const;
	const VTLine& GetStructuredLine(int line) const;
	
	// Cell access for deep parsing
	const VTCell& GetCell(int col, int row) const;
	
	void Clear();
	void RunTest();
	
	Size GetSize() const { return size; }

	Mutex& GetMutex() { return mutex; }

private:
	TerminalCtrl terminal;
	Size size;
	Mutex mutex;
};

}

#endif
