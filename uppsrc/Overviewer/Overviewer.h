#ifndef _Overviewer_Overviewer_h_
#define _Overviewer_Overviewer_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include "Settings.h"

using namespace Upp;

#define LAYOUTFILE <Overviewer/Overviewer.lay>
#include <CtrlCore/lay.h>

enum {
	FLAG_TEMPORARY = 1,
	FLAG_WRONG_LOCATION = 2,
	FLAG_WRONG_NAME = 4,
	FLAG_TOO_LARGE = 8,
	FLAG_NEEDS_REVIEW = 16,
	FLAG_CONTENT_NEEDS_REVIEW = 32
};

struct ListItem : Moveable<ListItem> {
	String text;
	bool done = false;
	String date;
	String commit;

	void Jsonize(JsonIO& jio) {
		jio("text", text)("done", done)("date", date)("commit", commit);
	}
};

struct FileMetadata : Moveable<FileMetadata> {
	uint32 flags = 0;
	int quality = 0;
	int completion = 0;
	int priority = 0;
	String notes;
	Vector<String> current_tags;
	Vector<String> reason_tags;
	Vector<String> gap_tags;
	Vector<ListItem> problems;
	Vector<ListItem> tasks;
	Vector<ListItem> leads;

	void Jsonize(JsonIO& jio) {
		jio("flags", (int&)flags)("quality", quality)("completion", completion)("priority", priority)
		   ("notes", notes)
		   ("current_tags", current_tags)("reason_tags", reason_tags)("gap_tags", gap_tags)
		   ("problems", problems)("tasks", tasks)("leads", leads);
	}
};

struct OverviewerProject {
	String path;
	String working_dir;
	int version = 1;
	VectorMap<String, FileMetadata> metadata;
	Vector<String> known_current_tags;
	Vector<String> known_reason_tags;
	Vector<String> known_gap_tags;
	
	void Jsonize(JsonIO& jio) {
		jio("version", version)("working_dir", working_dir)("metadata", metadata)
		   ("known_current_tags", known_current_tags)
		   ("known_reason_tags", known_reason_tags)
		   ("known_gap_tags", known_gap_tags);
	}
	
	void Reset() {
		path = "";
		working_dir = "";
		version = 1;
		metadata.Clear();
		known_current_tags.Clear();
		known_reason_tags.Clear();
		known_gap_tags.Clear();
	}
	
	FileMetadata GetEffectiveMetadata(const String& rel_path) const;
	String GetBackupPath() const;
	bool WriteBackup() const;
};

class SettingsWindow : public WithSettingsLayout<TopWindow> {
public:
	typedef SettingsWindow CLASSNAME;
	SettingsWindow();
	void Load();
	void Save();
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
	void OnNoteChange();

	void OnBatchEdit();
	void OnSettings();

	void SaveLayout();
	void LoadLayout();

	void CheckAutosave();
	void MarkSession(bool active);
	bool CheckRecovery();

public:
	struct FilterConfig {
		int mode = 0; // 0: all, 1: advanced
		uint32 flags = 0;
		int priority_min = 0;
		bool missing_priority = false;
		bool missing_completion = false;
		String tag_current, tag_reason, tag_gap;
	} filter;

private:
	OverviewerProject project;
	bool dirty = false;
	String current_selection;
	Time last_autosave;

	MenuBar menu;
	
	TreeCtrl tree;
	
	ParentCtrl flags_pane;
	Option temporary, wrong_location, wrong_name, too_large, needs_review, content_needs_review;
	
	ParentCtrl numeric_pane;
	DropList quality, completion, priority;
	
	ParentCtrl info_pane;
	Label path_lbl, type_lbl, size_lbl;

	DocEdit notes_editor;

	struct TagPanel : ParentCtrl {
		typedef TagPanel CLASSNAME;
		ArrayCtrl list;
		Button add, remove;
		Vector<String>* assigned;
		Vector<String>* global_known;
		Gate<> when_change;

		TagPanel(Vector<String>* a, Vector<String>* g) : assigned(a), global_known(g) {
			Add(list.SizePos());
			AddFrame(TopSeparatorFrame());
			AddFrame(BottomSeparatorFrame());
			list.AddColumn("Tag");
		}
		void Refresh();
		void OnAdd();
		void OnRemove();
	};

	struct ListPanel : ParentCtrl {
		typedef ListPanel CLASSNAME;
		ArrayCtrl list;
		Button add, edit, remove, toggle_done;
		Vector<ListItem>* items;
		Gate<> when_change;

		ListPanel(Vector<ListItem>* it) : items(it) {
			Add(list.SizePos());
			list.AddColumn("Done", 20);
			list.AddColumn("Text");
			list.AddColumn("Date");
			list.AddColumn("Commit");
		}
		void Refresh();
		void OnAdd();
		void OnEdit();
		void OnRemove();
		void OnToggleDone();
	};

	TagPanel current_tags_pane, reason_tags_pane, gap_tags_pane;
	ListPanel problems_pane, tasks_pane, leads_pane;

	DockableCtrl* dock_tree = nullptr;
	DockableCtrl* dock_flags = nullptr;
	DockableCtrl* dock_numeric = nullptr;
	DockableCtrl* dock_info = nullptr;
	DockableCtrl* dock_notes = nullptr;
	DockableCtrl* dock_current_tags = nullptr;
	DockableCtrl* dock_reason_tags = nullptr;
	DockableCtrl* dock_gap_tags = nullptr;
	DockableCtrl* dock_problems = nullptr;
	DockableCtrl* dock_tasks = nullptr;
	DockableCtrl* dock_leads = nullptr;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	
	void CreateFlagsPane();
	void CreateNumericPane();
	void CreateInfoPane();
	
	FileMetadata dummy_metadata; // for when nothing is selected
};

class BatchEditDialog : public TopWindow {
public:
	typedef BatchEditDialog CLASSNAME;
	BatchEditDialog(OverviewerProject& p, const String& start_path);

	struct FieldEdit : ParentCtrl {
		Option enable;
		Ctrl* ctrl;
		FieldEdit(const char* label, Ctrl& c);
	};

	ArrayCtrl list;
	Option recursive;
	
	Option apply_flags, apply_numeric, apply_tags;
	
	Option f_temp, f_loc, f_name, f_large, f_needs, f_content;
	DropList op_flags; // Add, Remove
	
	DropList quality, completion, priority;
	Option en_q, en_c, en_p;
	
	EditString tag_current, tag_reason, tag_gap;
	DropList op_tags; // Add, Remove
	Option en_tc, en_tr, en_tg;

	Button ok, cancel;

	OverviewerProject& project;
	String initial_path;

	void OnApply();
};

#endif
