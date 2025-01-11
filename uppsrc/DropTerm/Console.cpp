#include "DropTerm.h"

NAMESPACE_UPP

ConsoleCtrl::ConsoleCtrl() : cmd(*this) {
	SetView();
	
	#if 0
	AddProgram("ls",	THISBACK(ListFiles));
	AddProgram("cd",	THISBACK(ChangeDirectory));
	AddProgram("mkdir",	THISBACK(CreateDirectory));
	AddProgram("rm",	THISBACK(RemoveFile));
	AddProgram("cat",	THISBACK(ShowFile));
	AddProgram("edit",	THISBACK(EditFile));
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
		RemoveChild(active);
		active = 0;
	}
	
	if (ext)
		active = &*ext;
	else
		active = &cmd;
	
	if (active) {
		Add(active->SizePos());
		PostCallback([=]{
			this->LoadEditPos();
		});
	}
	
	WhenViewChange();
	WhenTitle();
}

void ConsoleCtrl::Menu(Bar& bar) {
	if (ext)
		ext->ToolMenu(bar);
	
	if (active && active != &cmd) {
		bar.Separator();
		bar.Add(AK_LEAVE_PROGRAM, THISBACK1(RemoveExt, false));
	}
}

void ConsoleCtrl::RemoveExt(bool fast_exit) {
	if (!ext) return;
	
	if (active == &*ext) {
		SaveEditPos();
		SaveEditPos.Clear();
		LoadEditPos.Clear();
		RemoveChild(&*ext);
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

void ConsoleCtrl::ListFiles(String arg) {
	
	// TODO: use arg
	
	// Calculate max line length
	Font fnt = cmd.GetFont();
	int width = cmd.GetSize().cx;
	int len4 = GetTextSize("abcd", fnt).cx;
	int max_chrs = width * 4 / len4;
	int max_fname = 30; // Max chars to show from filename
	int cols = max(1, max_chrs / max_fname);
	
	PutLine("Listing all files in " + cwd);
	FindFile ff(AppendFileName(cwd, "*"));
	bool first = true;
	String line;
	int col = 0;
	while(ff) {
		String fname = ff.GetName();
		if (fname.GetCharCount() > max_fname - 1)
			fname = fname.Left(max_fname - 1) + " ";
		else
			fname.Cat(' ', max_fname - fname.GetCharCount());
		line += fname;
		col++;
		if (col == cols) {
			PutLine(line);
			col = 0;
			line = "";
		}
		ff.Next();
	}
	if (line != "")
		PutLine(line);
}

void ConsoleCtrl::ChangeDirectory(String arg) {
	
}

void ConsoleCtrl::CreateDirectory(String arg) {
	
}

void ConsoleCtrl::RemoveFile(String arg) {
	
}

void ConsoleCtrl::ShowFile(String arg) {
	
}

void ConsoleCtrl::EditFile(String arg) {
	this->CreateExt<Word>();
	SetView();
}

void ConsoleCtrl::DownloadFile(String arg) {
	
}

END_UPP_NAMESPACE
