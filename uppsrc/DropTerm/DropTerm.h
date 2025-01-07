#ifndef _DropTerm_DropTerm_h
#define _DropTerm_DropTerm_h

#include <signal.h>
#include <CtrlLib/CtrlLib.h>
#include <TabBar/TabBar.h>
#include <PtyProcess/PtyProcess.h>
#include <RichEdit/RichEdit.h>
#include <PdfDraw/PdfDraw.h>
#include <CodeEditor/CodeEditor.h>
#include <Esc/Esc.h>
using namespace Upp;


#ifdef flagHAVE_INTRANET
#include <Intranet/Intranet.h>
#endif

#define KEYGROUPNAME "Dropdown Terminal"
#define KEYNAMESPACE Dropdown
#define KEYFILE      <DropTerm/DropTerm.key>
#include             <CtrlLib/key_header.h>

using namespace Dropdown;

#define IMAGECLASS DropTermImg
#define IMAGEFILE <DropTerm/DropTerm.iml>
#include <Draw/iml_header.h>

#include "CommandPrompt.h"
#include "Word.h"
#include "Console.h"

class TrayApp : public TrayIcon {
	bool is_exit;
	
public:
	typedef TrayApp CLASSNAME;

	TrayApp();
	
	bool IsExit() const {return is_exit;}
	void Exit() {is_exit = true; Break();}
	void Close() {Break();}
	
	virtual void LeftDouble();
	virtual void LeftDown();
	virtual void Menu(Bar& bar);
};


class DropTerm : public TopWindow {
	MenuBar						menu;
	
protected:
	ArrayMap<int, ConsoleCtrl>	cons;
	TabBar						tabs;
	double						alpha = 0.9;
	int							id_counter;
	bool						def_terminal = true;
	
	void ShowTabId(int i);
	void ShowTab();
	
public:
	typedef DropTerm CLASSNAME;
	DropTerm();
	
	ConsoleCtrl& NewConsole();
	ConsoleCtrl* GetActiveConsole();
	void SetSemiTransparent();
	void Quit() {PostClose();}
	void AddConsole() {NewConsole();}
	void CloseTab();
	void CloseTabId(int id);
	void PreviousTab();
	void NextTab();
	void TabClosed(Value tab);
	void MainMenu(Bar& menu);
	void AppMenu(Bar& menu);
	void ViewMenu(Bar& menu);
	void SetupMenu(Bar& menu);
	void RefreshMenu();
	void SetLang(int lang);
	void RefreshTitle(int id);
	void RefreshTitle1(String title, int id);
	void ViewChange(int id);
	void RemoveId(int id);
	
	virtual bool Key(dword key, int count);

	void PostClose() {PostCallback(THISBACK(Close0));}
	void Close0() {Close();}
	void PostTopMost() {PostCallback(THISBACK(TopMost0));}
	void TopMost0();
	void SetLayout();
	void PostSetLayout() {PostCallback(THISBACK(SetLayout));}
	void PostSetSemiTransparent() {PostCallback(THISBACK(SetSemiTransparent));}
	
};



class IdeDropdownTerminal {
	bool enable_ftpd = false;
	DropTerm cons;
	dword key;
	bool enabled = false;
	bool is_tray;
	bool is_cons_toggled;
	bool is_exit = false;
	bool is_fallback_theide = false;
	TrayApp* last_tray;
	
public:
	typedef IdeDropdownTerminal CLASSNAME;
	IdeDropdownTerminal();
	~IdeDropdownTerminal();
	void ToggleWindow();
	void Run();
	bool IsEnabled() const {return enabled;}
	bool IsRunning() const {return !is_exit;}
	bool IsIdeFallback() const {return is_fallback_theide;}
	void SetIdeFallback(bool b=true) {is_fallback_theide = b;}
	
};

#endif
