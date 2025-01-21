#include "Shell.h"

NAMESPACE_UPP

ConsoleCtrl::ConsoleCtrl() : cmd(*this) {
	SetView();
	
	AddProgram("help",	THISBACK(SimpleExt<Widget::DraftPad>));
	AddProgram("draft",	THISBACK(SimpleExt<Widget::DraftPad>));
	AddProgram("timer",	THISBACK(SimpleExt<Widget::Timer>));
	#if 0
	AddProgram("mkdir",	THISBACK(CreateDirectory));
	AddProgram("rm",	THISBACK(RemoveFile));
	AddProgram("cat",	THISBACK(ShowFile));
	AddProgram("get",	THISBACK(DownloadFile));
	#endif
	
	cwd = GetHomeDirectory();
}

ConsoleCtrl::~ConsoleCtrl() {
	RemoveExt(true);
}

bool ConsoleCtrl::RealizeFocus() {
	if (active && !active->HasFocusDeep()) {
		active->SetFocus();
		return true;
	}
	return false;
}

void ConsoleCtrl::SetView() {
	if (active) {
		if (internal_menubar && ext && active == &*ext)
			RemoveMenuBar();
		RemoveChild(active);
		active = 0;
	}
	
	if (ext)
		active = &*ext;
	else
		active = &cmd;
	
	if (active) {
		if (internal_menubar && ext && active == &*ext)
			AddMenuBar();
		Add(active->SizePos());
		PostCallback([=]{
			this->LoadEditPos();
		});
	}
	
	WhenViewChange();
	WhenTitle();
}

void ConsoleCtrl::AddMenuBar() {
	AddFrame(menu);
	menu.Set([this](Bar& b) {
		b.Add(Shell::AK_LEAVE_PROGRAM, THISBACK1(RemoveExt, false));
		ext->ToolMenu(b);
	});
}

void ConsoleCtrl::RemoveMenuBar() {
	RemoveFrame(menu);
}

void ConsoleCtrl::Menu(Bar& bar) {
	if (ext)
		ext->ToolMenu(bar);
	
	if (active && active != &cmd) {
		bar.Separator();
		bar.Add(Shell::AK_LEAVE_PROGRAM, THISBACK1(RemoveExt, false));
	}
}

void ConsoleCtrl::RemoveExt(bool fast_exit) {
	if (!ext) return;
	
	if (active == &*ext) {
		SaveEditPos();
		SaveEditPos.Clear();
		LoadEditPos.Clear();
		RemoveChild(&*ext);
		RemoveMenuBar();
		ext.Clear();
		active = 0;
		if (!fast_exit)
			SetView();
	}
	else ext.Clear();
}

String ConsoleCtrl::GetTitle() {
	String s;
	if (ext) {
		s = ext->GetTitle();
		if (s.IsEmpty())
			s = "App";
	}
	else
		s = "Console";
	return s;
}

END_UPP_NAMESPACE
