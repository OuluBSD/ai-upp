#ifndef _DropTerm_DropTerm_h
#define _DropTerm_DropTerm_h

#include <signal.h>
#include <CtrlLib/CtrlLib.h>
#include <TabBar/TabBar.h>
#include <Terminal/Terminal.h>
#include <PtyProcess/PtyProcess.h>
#include <RichEdit/RichEdit.h>
#include <PdfDraw/PdfDraw.h>
#include <CodeEditor/CodeEditor.h>
#include <Esc/Esc.h>
#include "InternalShell.h"

#ifndef flagGUI
	#error GUI flag is required
#endif

#ifndef flagV1
	#include <AI/Ctrl/Ctrl.h>
#endif
#include <ide/Shell/Shell.h>
using namespace Upp;


#ifdef flagNET
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

#include "Word.h"

NAMESPACE_UPP

#ifdef flagOSX
class TrayApp : public TopWindow
#else
class TrayApp : public TrayIcon
#endif
{
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


// TerminalCtrl with PtyProcess integration
class TerminalConsole : public TerminalCtrl {
public:
	typedef TerminalConsole CLASSNAME;
	
private:
	int bridge_id = -1;
	Value when_title_data;
	
	// For system shell
	PtyProcess pty;
	bool use_internal_shell = false;
	
	// For internal shell
	InternalShell internal_shell;
	String input_buffer;
	
public:
	TerminalConsole(bool internal_shell = false) : use_internal_shell(internal_shell) {
		InlineImages().Hyperlinks().WindowOps();
		WhenBell = [=]() { BeepExclamation(); };
		
		if (use_internal_shell) {
			// Setup for internal shell
			WriteUtf8(internal_shell.GetPrompt());
		} else {
			// Setup for system shell (PtyProcess)
			WhenOutput = [=](String s) { pty.Write(s); };
			WhenResize = [=]() { pty.SetSize(GetPageSize()); };
			
			// Start with system shell
			pty.Start(GetEnv("SHELL"), Environment(), GetHomeDirectory());
		}
	}
	
	bool Do() {
		if (use_internal_shell) {
			// Process internal shell
			return true; // Always running for internal shell
		} else {
			// Process system shell
			WriteUtf8(pty.Get());
			return pty.IsRunning();
		}
	}
	
	virtual bool Key(dword key, int count) override {
		if (use_internal_shell) {
			// Handle key input for internal shell
			if (key == K_ENTER) {
				// Process the input buffer
				String result = internal_shell.ProcessInput(input_buffer);
				WriteUtf8(result);
				WriteUtf8(internal_shell.GetPrompt());
				input_buffer.Clear();
			} else if (key == K_BACKSPACE || key == (K_BACKSPACE|K_SHIFT)) {
				if (input_buffer.GetCount() > 0) {
					input_buffer.Trim(input_buffer.GetCount() - 1);
					// For now, just show a simple backspace in the terminal
					WriteUtf8("\b \b");
				}
			} else if (key >= 32 && key <= 126) { // Printable characters
				char ch = (char)(key & 0xFF);
				input_buffer.Cat(ch);
				WriteUtf8(String(ch));
			}
			return true;
		} else {
			return TerminalCtrl::Key(key, count);
		}
	}
	
	void SetBridgeId(int id) { bridge_id = id; }
	int GetBridgeId() const { return bridge_id; }
	
	void SetWhenTitleData(const Value& data) { when_title_data = data; }
	Value GetWhenTitleData() const { return when_title_data; }
	
	bool IsUsingInternalShell() const { return use_internal_shell; }
};

class DropTerm : public TopWindow {
	MenuBar						menu;
	
protected:
	friend class IdeDropdownTerminal;
	ArrayMap<int, TerminalConsole> cons;  // Changed from ConsoleCtrl to TerminalConsole
	TabBar						tabs;
	double						alpha = 0.9;  // Transparency level (0.0 = fully transparent, 1.0 = opaque)
	int							id_counter;
	bool						def_terminal = true;
	bool						is_exit = false;
	bool						is_visible = false;  // Track visibility state
	
	void ShowTabId(int i);
	void ShowTab();
	
public:
	typedef DropTerm CLASSNAME;
	DropTerm();
	
	TerminalConsole& NewConsole(bool use_internal_shell = false);  // Changed return type and added parameter
	TerminalConsole* GetActiveConsole();  // Changed return type
	void SetSemiTransparent();
	void Quit();
	void LeaveProgram();
	void AddConsole(bool use_internal_shell = false) {NewConsole(use_internal_shell);}
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
	void AddInternalShell();
	bool IsExit() const {return is_exit;}
	
	virtual bool Key(dword key, int count);

	void PostClose() {PostCallback(THISBACK(Close0));}
	void Close0() {Close(); is_visible = false; }
	void PostTopMost() {PostCallback(THISBACK(TopMost0));}
	void TopMost0();
	void SetLayout();
	void PostSetLayout() {PostCallback(THISBACK(SetLayout));}
	void PostSetSemiTransparent() {PostCallback(THISBACK(SetSemiTransparent));}
	
	// Visibility methods
	void ShowWindow();
	void HideWindow();
	bool IsWindowVisible() const { return is_visible; }
	
	// Transparency methods
	void SetTransparency(double level);  // Level from 0.0 (fully transparent) to 1.0 (opaque)
	double GetTransparency() const { return alpha; }
	void SetSemiTransparent();
};


class IdeDropdownTerminal {
	bool enable_intranet = false;
	DropTerm cons;
	dword toggle_key = 0;
	dword ide_key = 0;
	bool enabled = false;
	bool is_tray = false;
	bool is_cons_toggled = false;
	bool is_ide_toggled = false;
	bool is_exit = false;
	One<TrayApp> tray;
	
public:
	typedef IdeDropdownTerminal CLASSNAME;
	IdeDropdownTerminal();
	~IdeDropdownTerminal();
	void Init();
	void ToggleWindow();
	void ToggleIde();
	void Reset();
	void Run();
	bool IsEnabled() const {return enabled && !is_ide_toggled;}
	bool IsRunning() const {return !is_exit;}
	bool IsIdeToggled() const {return is_ide_toggled;}
	
};

END_UPP_NAMESPACE

#endif
