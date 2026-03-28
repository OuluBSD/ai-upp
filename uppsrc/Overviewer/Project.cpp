#include "Overviewer.h"

OverviewerWindow::OverviewerWindow() {
	Title("Overviewer");
	Sizeable().Zoomable();

	AddFrame(menu);
	menu.Set(THISBACK(MainMenu));

	placeholder.SetLabel("Future Overview Area Placeholder");
	placeholder.SetAlign(ALIGN_CENTER);
	Add(placeholder.SizePos());
}

void OverviewerWindow::SyncTitle() {
	String t = "Overviewer";
	if (project.path.IsEmpty())
		t << " - (Untitled)";
	else
		t << " - " << project.path;
	if (dirty)
		t << " *";
	Title(t);
}

void OverviewerWindow::New() {
	if (!ConfirmSave()) return;
	project.Reset();
	ClearDirty();
}

void OverviewerWindow::Open() {
	if (!ConfirmSave()) return;
	FileSel fs;
	fs.Type("Project File", "*.json");
	if (fs.ExecuteOpen()) {
		OpenFile(fs.Get());
	}
}

void OverviewerWindow::OpenFile(const String& path) {
	String content = LoadFile(path);
	if (content.IsEmpty()) {
		Exclamation("Failed to load project file.");
		return;
	}
	try {
		OverviewerProject p;
		LoadFromJson(p, content);
		project = p;
		project.path = path;
		ClearDirty();
	} catch (const Exc& e) {
		Exclamation("Failed to parse project file: " + e);
	}
}

void OverviewerWindow::Save() {
	if (project.path.IsEmpty()) {
		SaveAs();
		return;
	}
	String json = StoreAsJson(project);
	if (!SaveFile(project.path, json)) {
		Exclamation("Failed to save project file.");
		return;
	}
	ClearDirty();
}

void OverviewerWindow::SaveAs() {
	FileSel fs;
	fs.Type("Project File", "*.json");
	if (fs.ExecuteSaveAs()) {
		project.path = fs.Get();
		Save();
	}
}

bool OverviewerWindow::ConfirmSave() {
	if (!dirty) return true;
	int res = PromptYesNoCancel("Save changes to project?");
	if (res == 1) {
		Save();
		return !dirty;
	}
	return res == 0;
}

void OverviewerWindow::Exit() {
	if (ConfirmSave()) {
		Break();
	}
}

void OverviewerWindow::Close() {
	if (ConfirmSave()) {
		TopWindow::Close();
	}
}

void OverviewerWindow::DockInit() {
}

void OverviewerWindow::MainMenu(Bar& bar) {
	bar.Add("File", THISBACK(FileMenu));
	bar.Add("Edit", THISBACK(EditMenu));
	bar.Add("View", THISBACK(ViewMenu));
	bar.Add("Help", THISBACK(HelpMenu));
}

void OverviewerWindow::FileMenu(Bar& bar) {
	bar.Add("New", THISBACK(New));
	bar.Add("Open", THISBACK(Open));
	bar.Add("Save", THISBACK(Save));
	bar.Add("Save As", THISBACK(SaveAs));
	bar.Separator();
	bar.Add("Exit", THISBACK(Exit));
}

void OverviewerWindow::EditMenu(Bar& bar) {
	bar.Add("Mark Dirty (debug)", THISBACK(MarkDirty));
}

void OverviewerWindow::ViewMenu(Bar& bar) {
}

void OverviewerWindow::HelpMenu(Bar& bar) {
	bar.Add("About", [] { PromptOK("Overviewer Milestone 1"); });
}
