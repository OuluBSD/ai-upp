#include "DropTerm.h"

#define KEYGROUPNAME "Dropdown Terminal"
#define KEYNAMESPACE Dropdown
#define KEYFILE      <DropTerm/DropTerm.key>
#include             <CtrlLib/key_source.h>

DropTerm::DropTerm() {
	Icon(DropTermImg::icon());
	FrameLess();
	
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

ConsoleCtrl& DropTerm::NewConsole() {
	int id = id_counter++;
	ConsoleCtrl& c = cons.Add(id);
	c.SetBridge(this, id);
	c.WhenTitle << THISBACK1(RefreshTitle, id);
	c.WhenViewChange << THISBACK1(ViewChange, id);
	tabs.AddKey(id, "Console", DropTermImg::icon(), Null, true);
	ShowTabId(id);
	return c;
}

void DropTerm::CloseTab() {
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
	if (active < 0) active += tabs.GetCount();
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
		Ctrl& c = cons[i].SizePos();
		Add(c);
		c.SetFocus();
	}
}

void DropTerm::RemoveId(int id) {
	int i;
	if ((i = cons.Find(id)) >= 0)
		cons.Remove(i);
}

void DropTerm::TabClosed(Value tab) {
	int i, id = tab;
	
	if ((i = cons.Find(i)) >= 0) {
		RemoveChild(&cons[i]);
		cons.Remove(i);
	}
}

ConsoleCtrl* DropTerm::GetActiveConsole() {
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
	
	ConsoleCtrl* cons = GetActiveConsole();
	if (cons){
		String cons_menu = cons->GetMenuTitle();
		if (!cons_menu.IsEmpty()) {
			menu.Add(cons_menu, callback(cons, &ConsoleCtrl::Menu));
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
	ConsoleCtrl& c = cons.Get(id);
	String title = c.GetTitle();
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
	menu.Add(AK_OPENCONS, THISBACK(AddConsole));
	menu.Add(AK_QUIT, THISBACK(Quit))
	#ifdef flagDEBUG
		.Key(K_CTRL_Q)
	#endif
	;
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
	if (key == (K_LEFT|K_CTRL)) {
		PreviousTab();
		return true;
	}
	if (key == (K_RIGHT|K_CTRL)) {
		NextTab();
		return true;
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
	SetRect(scr.left, 1, sz.cx - 1, sz.cy * 3 / 5);
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






#define IMAGECLASS DropTermImg
#define IMAGEFILE <DropTerm/DropTerm.iml>
#include <Draw/iml_source.h>

#if defined flagDEBUG && !__MINGW32__
	#define DEBUG_APP_PROFILE 1
#endif

volatile sig_atomic_t hupflag = 0;

extern "C" void hangup(int) {
	hupflag = 1;
}



void GlobalToggleWindow(IdeDropdownTerminal* term) {
	term->ToggleWindow();
}

void IdeDropdownTerminal::ToggleWindow() {
	is_cons_toggled = true;
	if (is_tray)
		last_tray->Close();
	else
		cons.PostClose();
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




void FtpDaemonService() {
	FTPServer f;
	f.Start();
	while(!Thread::IsShutdownThreads())
		Sleep(500);
}

IdeDropdownTerminal::IdeDropdownTerminal() {
	#if defined flagPOSIX
	signal(SIGHUP, hangup);
	#elif defined flagWIN32
	signal(SIGBREAK, hangup);
	#endif
	
	
	enabled = FindIndex(CommandLine(), "--dropdown-terminal") >= 0;
	
	enable_ftpd = FindIndex(CommandLine(), "--ftpd") >= 0;
	#ifdef flagHAVE_INTRANET
	if (enable_ftpd)
		Thread::Start(FtpDaemonService);
	#endif
	
	LoadKeys();
	cons.AddConsole();
	
	
	#if DEBUG_APP_PROFILE
	key = K_CTRL|K_SHIFT|K_X;
	#elif defined flagPOSIX
	key = 0xa7 | K_DELTA; // §
	is_tray = true;
	#else
	key = 65756;
	is_tray = true;
	#endif
	
	Ctrl::RegisterSystemHotKey(key, callback1(GlobalToggleWindow, this));
	
	
	is_cons_toggled = true;
	
}

IdeDropdownTerminal::~IdeDropdownTerminal() {
	SaveKeys();
}

void IdeDropdownTerminal::Run() {
	if (!is_cons_toggled)
		return;
	is_cons_toggled = false;
	
	if (is_tray) {
		TrayApp tray;
		last_tray = &tray;
		tray.Run();
		is_exit = tray.IsExit();
		is_cons_toggled = !is_exit;
	}
	else {
		#if !DEBUG_APP_PROFILE
		cons.PostTopMost();
		#endif
		cons.Run();
		cons.CloseTopCtrls();
		SaveKeys();
	}
	
	is_tray = !is_tray;
	
	#if DEBUG_APP_PROFILE
	is_exit = true;
	#endif
	
	if (hupflag)
		is_exit = true;
}
