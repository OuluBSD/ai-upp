#ifndef _Overviewer_Overviewer_h_
#define _Overviewer_Overviewer_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>

using namespace Upp;

struct OverviewerProject {
	String path;
	String working_dir;
	int version = 1;
	
	void Jsonize(JsonIO& jio) {
		jio("version", version)("working_dir", working_dir);
	}
	
	void Reset() {
		path = "";
		working_dir = "";
		version = 1;
	}
};

class OverviewerWindow : public DockWindow {
public:
	typedef OverviewerWindow CLASSNAME;
	OverviewerWindow();

	void New();
	void Open();
	void OpenFile(const String& path);
	void Save();
	void SaveAs();
	void Exit();

	void MarkDirty() { dirty = true; SyncTitle(); }
	void ClearDirty() { dirty = false; SyncTitle(); }
	bool IsDirty() const { return dirty; }

	void SyncTitle();
	bool ConfirmSave();

	virtual void Close() override;
	virtual void DockInit() override;

private:
	OverviewerProject project;
	bool dirty = false;
	
	MenuBar menu;
	Label placeholder;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
};

#endif
