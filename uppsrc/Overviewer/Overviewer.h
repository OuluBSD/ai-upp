#ifndef _Overviewer_Overviewer_h_
#define _Overviewer_Overviewer_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>

using namespace Upp;

enum {
	FLAG_TEMPORARY = 1,
	FLAG_WRONG_LOCATION = 2,
	FLAG_WRONG_NAME = 4,
	FLAG_TOO_LARGE = 8,
	FLAG_NEEDS_REVIEW = 16,
	FLAG_CONTENT_NEEDS_REVIEW = 32
};

struct FileMetadata : Moveable<FileMetadata> {
	uint32 flags = 0;
	int quality = 0;
	int completion = 0;
	int priority = 0;

	void Jsonize(JsonIO& jio) {
		jio("flags", (int&)flags)("quality", quality)("completion", completion)("priority", priority);
	}
};

struct OverviewerProject {
	String path;
	String working_dir;
	int version = 1;
	VectorMap<String, FileMetadata> metadata;
	
	void Jsonize(JsonIO& jio) {
		jio("version", version)("working_dir", working_dir)("metadata", metadata);
	}
	
	void Reset() {
		path = "";
		working_dir = "";
		version = 1;
		metadata.Clear();
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

	void RefreshTree();
	void OnTreeSelection();
	void UpdatePanels();
	void OnMetadataChange();

private:
	OverviewerProject project;
	bool dirty = false;
	String current_selection;
	int filter_mode = 0; // 0: all, 1: any flag, 2: priority 5

	MenuBar menu;
	
	TreeCtrl tree;
	
	ParentCtrl flags_pane;
	Option temporary, wrong_location, wrong_name, too_large, needs_review, content_needs_review;
	
	ParentCtrl numeric_pane;
	DropList quality, completion, priority;
	
	ParentCtrl info_pane;
	Label path_lbl, type_lbl, size_lbl;

	DockableCtrl* dock_tree = nullptr;
	DockableCtrl* dock_flags = nullptr;
	DockableCtrl* dock_numeric = nullptr;
	DockableCtrl* dock_info = nullptr;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	
	void CreateFlagsPane();
	void CreateNumericPane();
	void CreateInfoPane();
};

#endif
