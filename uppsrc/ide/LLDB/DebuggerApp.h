#ifndef _ide_LLDB_DebuggerApp_h_
#define _ide_LLDB_DebuggerApp_h_


class LLDBDebuggerApp : public TopWindow {
	Splitter hsplit, lsplit, csplit, bsplit, rsplit;
	TreeCtrl files;
	TabCtrl file_tabs;
	TabCtrl btabs;
	WithConsoleLayout<Ctrl> console;
	DocEdit normal, error;
	ArrayMap<String,CodeEditor> codes;
	WithThreadsLayout<Ctrl> threads;
	TabCtrl loctabs, breaktabs;
	ArrayCtrl stack, locals, regs, breaks, watchs;
	DocEdit dbg_stream;
public:
	typedef LLDBDebuggerApp CLASSNAME;
	LLDBDebuggerApp();
	
	void Data();
	void DataLocalRecursive(lldb::SBValue local);
	
};


#endif
