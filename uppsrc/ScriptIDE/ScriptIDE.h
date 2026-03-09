#ifndef _ScriptIDE_ScriptIDE_h_
#define _ScriptIDE_ScriptIDE_h_

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <TabBar/TabBar.h>
#include <ByteVM/ByteVM.h>
#include <RichEdit/RichEdit.h>
#include <Docking/Docking.h>

namespace Upp {

template <class T>
class WithDockable : public DockableCtrl {
public:
	T ctrl;
	WithDockable() { Add(ctrl.SizePos()); }
	T* operator->() { return &ctrl; }
	operator T&() { return ctrl; }
};

#define LAYOUTFILE <ScriptIDE/ScriptIDE.lay>
#include <CtrlCore/lay.h>

/*
#define IMAGECLASS ScriptIDEImg
#define IMAGEFILE  <ScriptIDE/ScriptIDE.iml>
#include <Draw/iml_header.h>
*/

#include "Settings.h"
#include "VariableExplorer.h"
#include "PlotsPane.h"
#include "DebuggerPane.h"
#include "ProfilerPane.h"
#include "FindInFilesPane.h"
#include "OutlinePane.h"
#include "RunManager.h"
#include "Linter.h"
#include "PathManager.h"
#include "PathManagerDlg.h"
#include "PythonConsole.h"
#include "FilesPane.h"
#include "CustomFileTabs.h"
#include "PythonIDE.h"

}

#endif
