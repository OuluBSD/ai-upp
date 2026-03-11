#ifndef _ScriptIDE_Icons_h_
#define _ScriptIDE_Icons_h_

#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

Image GetTablerIcon(const char *name, int size = 24);

struct TablerIcons {
	static Image NewFile()    { return GetTablerIcon("file-plus"); }
	static Image OpenFile()   { return GetTablerIcon("folder-open"); }
	static Image Save()       { return GetTablerIcon("device-floppy"); }
	static Image SaveAll()    { return GetTablerIcon("file-stack"); }
	static Image Run()        { return GetTablerIcon("player-play"); }
	static Image Debug()      { return GetTablerIcon("bug"); }
	static Image Stop()       { return GetTablerIcon("player-stop"); }
	static Image StepOver()   { return GetTablerIcon("arrow-right"); }
	static Image StepIn()     { return GetTablerIcon("arrow-down"); }
	static Image StepOut()    { return GetTablerIcon("arrow-up"); }
	static Image Breakpoint() { return GetTablerIcon("circle-dot"); }
	static Image Settings()   { return GetTablerIcon("settings"); }
	static Image Search()     { return GetTablerIcon("search"); }
	static Image Help()       { return GetTablerIcon("help"); }
	static Image Info()       { return GetTablerIcon("info-circle"); }
	static Image History()    { return GetTablerIcon("history"); }
	static Image VariableExplorer() { return GetTablerIcon("list-search"); }
	static Image Plots()      { return GetTablerIcon("chart-bar"); }
	static Image Profiler()   { return GetTablerIcon("analyze"); }
	static Image Outline()    { return GetTablerIcon("menu-2"); }
	static Image Files()      { return GetTablerIcon("files"); }
	static Image Folder()     { return GetTablerIcon("folder"); }
	static Image File()       { return GetTablerIcon("file"); }
	static Image Python()     { return GetTablerIcon("brand-python"); }
	static Image Maximize()   { return GetTablerIcon("arrows-maximize"); }
	static Image Plus()       { return GetTablerIcon("plus"); }
	static Image Minus()      { return GetTablerIcon("minus"); }
	static Image Undo()       { return GetTablerIcon("arrow-back-up"); }
	static Image Redo()       { return GetTablerIcon("arrow-forward-up"); }
};

END_UPP_NAMESPACE

#endif
