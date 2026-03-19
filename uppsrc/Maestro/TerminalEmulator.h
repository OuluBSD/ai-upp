#ifndef _Maestro_TerminalEmulator_h_
#define _Maestro_TerminalEmulator_h_

#include <Terminal/Parser.h>
#include <Terminal/Page.h>

namespace Upp {

class TerminalEmulator {
public:
	TerminalEmulator();
	
	void Write(const String& data);
	void SetSize(Size sz);
	
	VTPage& GetPage() { return page; }
	const VTPage& GetPage() const { return page; }

private:
	VTPage page;
	VTInStream parser;
	VTCell cellattrs;
	
	void PutChar(int c);
	void DispatchCtl(byte c);
	void DispatchEsc(const VTInStream::Sequence& seq);
	void DispatchCsi(const VTInStream::Sequence& seq);
	
	// Command implementations (subset of TerminalCtrl)
	void ClearPage(const VTInStream::Sequence& seq);
	void ClearLine(const VTInStream::Sequence& seq);
	void SelectGraphicsRendition(const VTInStream::Sequence& seq);
};

}

#endif
