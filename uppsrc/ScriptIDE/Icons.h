#ifndef _ScriptIDE_Icons_h_
#define _ScriptIDE_Icons_h_

#include <CtrlLib/CtrlLib.h>

NAMESPACE_UPP

Image GetIcon(const char *name, int size = 24);

struct Icons {
	static Image NewFile()    { return GetIcon("project_new"); }
	static Image OpenFile()   { return GetIcon("project_open"); }
	static Image Save()       { return GetIcon("filesave"); }
	static Image SaveAll()    { return GetIcon("save_all"); }
	static Image Run()        { return GetIcon("run_selection"); }
	static Image Debug()      { return GetIcon("debug"); }
	static Image Stop()       { return GetIcon("project_close"); }
	static Image StepOver()   { return GetIcon("debug_cell"); }
	static Image StepIn()     { return GetIcon("debug_selection"); }
	static Image StepOut()    { return GetIcon("run_again"); }
	static Image Breakpoint() { return GetIcon("list_breakpoints"); }
	static Image Settings()   { return GetIcon("genprefs"); }
	static Image Search()     { return GetIcon("findnext"); }
	static Image Help()       { return GetIcon("spyder_about"); }
	static Image Info()       { return GetIcon("spyder_about"); }
	static Image History()    { return GetIcon("last_edit_location"); }
	static Image VariableExplorer() { return GetIcon("variable-explorer"); }
	static Image Plots()      { return GetIcon("plots"); }
	static Image Profiler()   { return GetIcon("profiler"); }
	static Image Outline()    { return GetIcon("syspath"); }
	static Image Files()      { return GetIcon("projects"); }
	static Image Folder()     { return GetIcon("project_open"); }
	static Image File()       { return GetIcon("file_type_tex"); }
	static Image Python()     { return GetIcon("spyder"); }
	static Image Maximize()   { return GetIcon("maximize"); }
	static Image Plus()       { return GetIcon("new_cell"); }
	static Image Minus()      { return GetIcon("editdelete"); }
	static Image Undo()       { return GetIcon("undo"); }
	static Image Redo()       { return GetIcon("redo"); }
	static Image ClearConsole() { return GetIcon("clear_console"); }
};

END_UPP_NAMESPACE

#endif
