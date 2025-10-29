#include "DropTerm.h"
#include <ide/ide.h>
#include <Core/Core.h>

#define KEYGROUPNAME "Dropdown Terminal"
#define KEYNAMESPACE Dropdown
#define KEYFILE      <DropTerm/DropTerm.key>
#include             <CtrlLib/key_source.h>


#define IMAGECLASS DropTermImg
#define IMAGEFILE <DropTerm/DropTerm.iml>
#include <Draw/iml_source.h>

NAMESPACE_UPP

DropTerm::DropTerm() {
	Icon(DropTermImg::icon());
	FrameLess();
	// NoFocus(); // Don't focus when shown initially - This method doesn't exist, removing
	ToolWindow(); // Set as tool window to avoid taskbar
	
	id_counter = 0;
	
	AddFrame(menu);
	AddFrame(tabs);
	
	RefreshMenu();
	
	tabs.MinTabCount(0);
	tabs.Crosses();
	tabs.WhenClose << THISBACK(TabClosed);
	tabs.WhenAction << THISBACK(ShowTab);
	
	InitWord();
	PostSetLayout();
	PostSetSemiTransparent();
}

TerminalConsole& DropTerm::NewConsole(bool use_internal_shell) {
	int id = id_counter++;
	TerminalConsole& c = cons.Add(id); // Add with default constructor first
	c.Init(use_internal_shell); // Initialize with the internal shell setting
	c.SetBridgeId(id);  // Changed method call
	c.WhenTitle << [=](String s) { RefreshTitle(id); };  // Fixed lambda to not expect argument
	String tab_title = use_internal_shell ? "Internal Shell" : "Terminal";
	tabs.AddKey(id, tab_title, DropTermImg::icon(), Null, true);
	ShowTabId(id);
	return c;
}

void DropTerm::CloseTab() {
	if (!tabs.GetCount())
		return;
	int active = tabs.GetCursor();
	int tab_id = tabs.GetKey(active);
	CloseTabId(tab_id);
}

void DropTerm::CloseTabId(int id) {
	int active = tabs.GetCursor();
	for(int i = 0; i < tabs.GetCount(); i++) {
		int tab_id = tabs.GetKey(i);
		if (tab_id == id) {
			tabs.Close(i);
			if (tabs.GetCount() == 0)
				tabs.Clear();
			
			int new_active = active < tabs.GetCount() ? active : active - 1;
			int new_id = new_active >= 0 ? (int)tabs.GetKey(new_active) : -1;
			ShowTabId(new_id);
			
			RemoveId(id);
			break;
		}
	}
}

void DropTerm::PreviousTab() {
	int active = tabs.GetCursor() - 1;
	if (active < 0) active += tabs.GetCount();
	if (active < 0 || active >= tabs.GetCount()) return;
	tabs.SetCursor(active);
}

void DropTerm::NextTab() {
	int active = tabs.GetCursor() + 1;
	if (active >= tabs.GetCount()) active -= tabs.GetCount();
	if (active < 0 || active >= tabs.GetCount()) return;
	tabs.SetCursor(active);
}

void DropTerm::ShowTab() {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return;
	int id = tabs.GetKey(active);
	ShowTabId(id);
	RefreshMenu();
}

void DropTerm::ShowTabId(int id) {
	for(int i = 0; i < cons.GetCount(); i++)
		RemoveChild(&cons[i]);
	
	int i;
	if ((i = cons.Find(id)) >= 0) {
		TerminalConsole& c = cons[i];
		c.SizePos();
		Add(c);
		c.SetFocus();
	}
}

void DropTerm::RemoveId(int id) {
	int i;
	if ((i = cons.Find(id)) >= 0) {
		RemoveChild(&cons[i]);
		cons.Remove(i);
	}
}

void DropTerm::TabClosed(Value tab) {
	RemoveId(tab);
}

TerminalConsole* DropTerm::GetActiveConsole() {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return 0;
	int i = tabs.GetKey(active);
	int pos = cons.Find(i);
	return pos >= 0 ? &cons[pos] : 0;
}

void DropTerm::MainMenu(Bar& menu) {
	menu.Add(t_("App"), THISBACK(AppMenu));
	menu.Add(t_("View"), THISBACK(ViewMenu));
	menu.Add(t_("Setup"), THISBACK(SetupMenu));
	
	TerminalConsole* tcons = GetActiveConsole();
	if (tcons){
		String cons_menu = tcons->GetTitle();
		if (!cons_menu.IsEmpty()) {
			menu.Add(cons_menu, callback(tcons, &TerminalConsole::Menu));
		}
		return;
	}
}

void DropTerm::SetLang(int lang)
{
	SetLanguage(lang);
	RefreshMenu();
}

void DropTerm::RefreshTitle(int id) {
	TerminalConsole& c = cons.Get(id);
	String title = c.GetTitle();  // Using the new GetTitle method we added
	RefreshTitle1(title, id);
}

void DropTerm::RefreshTitle1(String title, int id) {
	for(int i = 0; i < tabs.GetCount(); i++) {
		if (tabs.GetKey(i) == id) {
			tabs.SetValue(i, title);
			break;
		}
	}
}

void DropTerm::ViewChange(int id) {
	int active = tabs.GetCursor();
	if (active < 0 || active >= tabs.GetCount()) return;
	active = tabs.GetKey(active);
	if (active != id) return;
	RefreshMenu();
}

void DropTerm::RefreshMenu() {
	menu.Clear();
	menu.Set(THISBACK(MainMenu));
}

void DropTerm::AppMenu(Bar& menu) {
	menu.Add(AK_OPENCONS, THISBACK1(AddConsole, false)); // Add console with system shell by default
	menu.Add(t_("Open Internal Shell"), THISBACK(AddInternalShell));
	menu.Add(Shell::AK_LEAVE_PROGRAM, THISBACK(LeaveProgram));
	menu.Add(AK_QUIT, THISBACK(Quit));
}

void DropTerm::AddInternalShell() {
	NewConsole(true);  // Create with internal shell
}

void DropTerm::ViewMenu(Bar& menu) {
	menu.Add(AK_CLOSETAB, THISBACK(CloseTab));
	menu.Add(AK_PREVTAB, THISBACK(PreviousTab));
	menu.Add(AK_NEXTTAB, THISBACK(NextTab));
}

void DropTerm::SetupMenu(Bar& menu)
{
	menu.Add(AK_ENGLISH, THISBACK1(SetLang, LNGC_('E','N','U','S', CHARSET_UTF8)))
	         .Radio(GetCurrentLanguage() == LNGC_('E','N','U','S', CHARSET_UTF8));
	menu.Add(AK_FINNISH, THISBACK1(SetLang, LNGC_('F','I','F','I', CHARSET_UTF8)))
	         .Radio(GetCurrentLanguage() == LNGC_('F','I','F','I', CHARSET_UTF8));
	menu.Separator();
	menu.Add(AK_KEYS, callback(EditKeys));
}

bool DropTerm::Key(dword key, int count) {
	if (key & K_DELTA && (key & K_CTRL || key & K_SHIFT || key & K_ALT)) {
		TopWindow* tw = GetTopWindow();
		if (tw && tw->HotKey(key))
			return true;
	}
	if (tabs.GetCount()) {
		int tab = tabs.GetCursor();
		int id = tabs.GetKey(tab);
		int i = cons.Find(id);
		if (i < 0) return false;
		TerminalConsole& ctrl = cons[i];
		// TerminalConsole doesn't have RealizeFocus() method, so just handle the key
		ctrl.SetFocus();
		return ctrl.Key(key,count);
	}
	return false;
}

void DropTerm::TopMost0() {
	TopMost();
	SetLayout();
	SetSemiTransparent();
}

void DropTerm::SetLayout() {
	Rect scr = GetPrimaryWorkArea();
	Size sz = scr.GetSize();
	SetRect(scr.left, scr.top+1, sz.cx - 1, sz.cy * 3 / 5);
}

void DropTerm::Quit() {
	PostClose();
	is_exit = true;
}

void DropTerm::LeaveProgram() {
	if (!tabs.GetCount())
		return;
	int active = tabs.GetCursor();
	int tab_id = tabs.GetKey(active);
	TerminalConsole& term = cons.Get(tab_id);
	if (term.IsUsingInternalShell()) {
		// For internal shell, maybe clear the terminal
		term.Clear();
	} else {
		// For system shell, the PtyProcess should be handled appropriately
		// TerminalConsole doesn't have RemoveExt method, so maybe clear content
		term.Clear();
	}
}

#ifdef flagWIN32
void DropTerm::SetSemiTransparent() {
	HWND h = GetTopWindow()->GetHWND();
	SetWindowLongA(h, GWL_EXSTYLE, WS_EX_LAYERED);
	SetLayeredWindowAttributes(h, 0, (BYTE)(alpha * 255), LWA_ALPHA);
}
#elif defined flagX11
void SetSemiTransparent0(double alpha, XDisplay* display, unsigned long win);
void DropTerm::SetSemiTransparent() {
	SetSemiTransparent0(alpha, Xdisplay, GetWindow());
}
#else
void DropTerm::SetSemiTransparent() {}
#endif

void DropTerm::SetTransparency(double level) {
	// Clamp the transparency level between 0.1 (nearly transparent) and 1.0 (opaque)
	if (level < 0.1) level = 0.1;
	if (level > 1.0) level = 1.0;
	alpha = level;
	
	// Apply the new transparency if the window is visible
	if (IsOpen() && IsVisible()) {
		SetSemiTransparent();
	}
}

void DropTerm::ShowWindow() {
	if (!is_visible) {
		TopWindow::Show();  // Use native show method
		is_visible = true;
		// Apply transparency when showing the window
		SetSemiTransparent();
	}
}

void DropTerm::HideWindow() {
	if (is_visible) {
		TopWindow::Hide();  // Use native hide method
		is_visible = false;
	}
}

















TrayApp::TrayApp() {
	Icon(DropTermImg::icon());
	Tip("This is U++ TrayIcon");
	
	is_exit = false;
	
}

void TrayApp::LeftDouble() {
	PromptOK("DropTerm is running!");
}

void TrayApp::LeftDown() {
	Close();
}

void TrayApp::Menu(Bar& bar) {
	bar.Add("Info..", THISBACK(LeftDouble));
	bar.Separator();
	bar.Add("Exit", THISBACK(Exit));
}





volatile sig_atomic_t hupflag = 0;

extern "C" void hangup(int) {
	hupflag = 1;
}



void GlobalToggleWindow(IdeDropdownTerminal* term) {
	term->ToggleWindow();
}

void GlobalToggleIde(IdeDropdownTerminal* term) {
	term->ToggleIde();
}

void IdeDropdownTerminal::ToggleWindow() {
	if (is_tray) {
		// Currently showing tray, need to show console
		if (tray) {
			tray->Close();
			tray.Clear();
		}
		// Show console window using native visibility
		cons.ShowWindow();
		is_tray = false;
		is_cons_toggled = false;  // Allow console to be shown in next Run() call
	}
	else {
		// Currently showing console, need to hide it using native visibility
		cons.HideWindow();
		is_cons_toggled = true;  // Prevent console from appearing until toggled again
		is_tray = true;   // Next Run() will show tray instead
	}
}

void IdeDropdownTerminal::ToggleIde() {
	if (!is_ide_toggled) {
		// Switching from IDE shown to IDE hidden (terminal should be visible)
		is_ide_toggled = true;
		// Need to hide the IDE and ensure terminal is shown
		if (TheIde()) {
			TheIde()->Close(); // Hide the IDE
		}
		// Make sure terminal is shown (not in tray mode)
		cons.ShowWindow();
		is_tray = false;
		is_cons_toggled = false;
	}
	else {
		// Switching from IDE hidden to IDE shown (terminal should hide)
		is_ide_toggled = false;
		// Hide the terminal and show the IDE again
		if (TheIde()) {
			TheIde()->Show(); // Show the IDE
		}
		cons.HideWindow(); // Hide the terminal using native visibility
		is_cons_toggled = true;
		is_tray = true;   // Next Run() will show tray instead
	}
}

String GetKeysFile() {
	return ConfigFile("keys.key");
}

void LoadKeys() {
	String keyfile = GetKeysFile();
	if (FileExists(keyfile)) {
		String s = LoadFile(keyfile);
		if (!s.IsEmpty())
			RestoreKeys(s);
	}
}

void SaveKeys() {
	FileOut fout(GetKeysFile());
	fout << StoreKeys();
	fout.Close();
}




IdeDropdownTerminal::IdeDropdownTerminal() {
	
}

void IdeDropdownTerminal::Init() {
	#if defined flagPOSIX
	signal(SIGHUP, hangup);
	#elif defined flagWIN32
	signal(SIGBREAK, hangup);
	#endif
	
	
	enabled = true;
	
	enable_intranet = FindIndex(CommandLine(), "--intranet") >= 0;
	#ifdef flagNET
	if (enable_intranet)
		Thread::Start(IntranetDaemon);
	#endif
	
	LoadKeys();
	cons.AddConsole();
	
	Index<int> nordic;
	nordic	<< LNGC_('D','A','D','K', CHARSET_UTF8)
			<< LNGC_('S','V','S','E', CHARSET_UTF8)
			<< LNGC_('F','I','F','I', CHARSET_UTF8)
			;
	#if 0
	int lng = GetCurrentLanguage();
	if (nordic.Find(lng) >= 0) {
		 // § key in danish, swedish and finnish keyboards
		#if defined flagPOSIX
		toggle_key = 0xa7 | K_DELTA;
		is_tray = true;
		#else
		toggle_key = 65756 | K_DELTA;
		#endif
		ide_key = toggle_key | K_SHIFT;
	}
	else {
		toggle_key = K_CTRL|K_SHIFT|K_X;
		ide_key = K_CTRL|K_SHIFT|K_C;
	}
	#else
	toggle_key = K_CTRL|K_SHIFT|K_X;
	ide_key = K_CTRL|K_SHIFT|K_C;
	#endif
	
	is_tray = true;
	Ctrl::RegisterSystemHotKey(toggle_key, callback1(GlobalToggleWindow, this));
	Ctrl::RegisterSystemHotKey(ide_key, callback1(GlobalToggleIde, this));
	
	
	is_cons_toggled = true;
}

IdeDropdownTerminal::~IdeDropdownTerminal() {
	SaveKeys();
}

void IdeDropdownTerminal::Reset() {
	is_cons_toggled = true;
	is_ide_toggled = false;
	is_exit = false;
}

void IdeDropdownTerminal::Run() {
	if (!is_cons_toggled)
		return;
	is_cons_toggled = false;
	
	if (is_tray) {
		tray.Create();
		tray->Run();
		is_exit = tray->IsExit();
		is_cons_toggled = !is_exit;
		tray->Close();
		tray.Clear();
	}
	else {
		#if !DEBUG_APP_PROFILE
		// Show the console window instead of running it directly
		cons.ShowWindow();
		#endif
		
		// Add a background thread to process terminal I/O
		// We'll run the thread only once to avoid creating multiple threads
		static bool termThreadStarted = false;
		static Thread termThread;
		
		if (!termThreadStarted) {
			termThreadStarted = true;
			termThread.Run([&]() {
				while(!cons.IsExit() && termThreadStarted) {
					for(int i = 0; i < cons.cons.GetCount(); i++) {
						TerminalConsole& term = cons.cons[i];
						term.Do(); // Process terminal I/O
					}
					Sleep(10); // Small delay to prevent excessive CPU usage
				}
			});
		}
		
		// The visibility is now handled via ShowWindow/HideWindow
		// Instead of running the console dialog directly, we just ensure it's visible
		is_exit = cons.IsExit();
		SaveKeys();
	}
	 
	is_tray = !is_tray;
	
	#if DEBUG_APP_PROFILE
	if (!cons.dbg_keep_running)
		is_exit = true;
	#endif
	
	if (hupflag)
		is_exit = true;
}

END_UPP_NAMESPACE
