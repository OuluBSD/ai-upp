#ifndef _ScriptIDE_ScriptIDE_h_
#define _ScriptIDE_ScriptIDE_h_

#include <CtrlLib/CtrlLib.h>
#include <CodeEditor/CodeEditor.h>
#include <TabBar/TabBar.h>
#include <ByteVM/ByteVM.h>
#include <RichEdit/RichEdit.h>
#include <Docking/Docking.h>
#include <FormEditor/FormEditor.h>
#include <ScriptCommon/ScriptCommon.h>

#define LAYOUTFILE <ScriptIDE/ScriptIDE.lay>
#include <CtrlCore/lay.h>

NAMESPACE_UPP

	inline void Todo(const String& msg) { PromptOK("Feature not yet implemented: " + msg); }
	inline void Todo() { Todo(""); }

	template <class T>
	class WithDockable : public DockableCtrl {
	public:
		T ctrl;
		WithDockable() { Add(ctrl.SizePos()); }
		T* operator->() { return &ctrl; }
		operator T&() { return ctrl; }
	};

#define IMAGECLASS ScriptIDEImg
#define IMAGEFILE  <ScriptIDE/ScriptIDE.iml>
#include <Draw/iml_header.h>

	// Forward declarations for circular dependencies
	class CustomFileTabs;
	class PythonConsole;
	class FilesPane;
	class VariableExplorer;
	class PlotsPane;
	class DebuggerPane;
	class ProfilerPane;
	class FindInFilesPane;
	class OutlinePane;
	class HelpPane;
	class HistoryPane;
	class GameStatePluginGUI;
	class CardGamePluginGUI;

#include "PluginInterfacesGUI.h"
#include "PluginManager.h"
#include "PreferencesPage.h"
#include "PythonIDE.h"

#include "PreferencesWindow.h"
#include "PythonEditor.h"
#include "Settings.h"
#include "VariableExplorer.h"
#include "PlotsPane.h"
#include "DebuggerPane.h"
#include "ProfilerPane.h"
#include "FindInFilesPane.h"
#include "OutlinePane.h"
#include "HelpPane.h"
#include "HistoryPane.h"
#include "CustomFileTabs.h"
#include "PythonConsole.h"
#include "FilesPane.h"
#include "PathManagerDlg.h"
#include "GameStatePlugin.h"
#include "CardGamePlugin.h"

END_UPP_NAMESPACE

#endif
