#ifndef _Overviewer_Overviewer_h_
#define _Overviewer_Overviewer_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include "Settings.h"
#include "OverviewGenerator.h"

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

struct Suggestion : Moveable<Suggestion> {
	String text;
	double confidence = 0.5;
	String source;
	bool rejected = false;

	void Jsonize(JsonIO& jio) {
		jio("text", text)("confidence", confidence)("source", source)("rejected", rejected);
	}
};

struct EntrySuggestions : Moveable<EntrySuggestions> {
	Vector<Suggestion> current_tags;
	Vector<Suggestion> reason_tags;
	Vector<Suggestion> gap_tags;
	Vector<Suggestion> problems;
	Vector<Suggestion> tasks;

	void Jsonize(JsonIO& jio) {
		jio("current_tags", current_tags)("reason_tags", reason_tags)("gap_tags", gap_tags)
		   ("problems", problems)("tasks", tasks);
	}
	
	void Clear() {
		current_tags.Clear(); reason_tags.Clear(); gap_tags.Clear();
		problems.Clear(); tasks.Clear();
	}
};

struct ReviewItem : Moveable<ReviewItem> {
	String path;
	String type;
	String message;
	int severity = 0; // 0: info, 1: warning, 2: error
	String source;
	bool dismissed = false;

	void Jsonize(JsonIO& jio) {
		jio("path", path)("type", type)("message", message)("severity", severity)("source", source)("dismissed", dismissed);
	}
};

struct HistoryEvent : Moveable<HistoryEvent> {
	Time time;
	String path;
	String type;
	String description;
	String old_value;
	String new_value;
	String source;

	void Jsonize(JsonIO& jio) {
		jio("time", time)("path", path)("type", type)("description", description)
		   ("old_value", old_value)("new_value", new_value)("source", source);
	}
};

struct EntryScore : Moveable<EntryScore> {
	double score = 0;
	Vector<String> factors;

	void Jsonize(JsonIO& jio) {
		jio("score", score)("factors", factors);
	}
};

struct ProjectDashboard {
	int total_files = 0;
	int total_dirs = 0;
	int flagged_entries = 0;
	int needs_review = 0;
	int missing_priority = 0;
	int missing_completion = 0;
	int with_notes = 0;
	int with_problems = 0;
	int with_tasks = 0;
	int with_leads = 0;
	int suggestions_pending = 0;
	int priority_counts[6];
	VectorMap<String, int> top_reason_tags;
	VectorMap<String, int> top_gap_tags;
	VectorMap<String, int> top_current_tags;
	
	int recent_changes = 0;
	Vector<String> recently_modified;
	int stale_entries = 0;
	
	VectorMap<String, double> top_action_items;

	ProjectDashboard() {
		total_files = total_dirs = flagged_entries = needs_review = 0;
		missing_priority = missing_completion = with_notes = with_problems = 0;
		with_tasks = with_leads = suggestions_pending = 0;
		recent_changes = stale_entries = 0;
		for(int i = 0; i < 6; i++) priority_counts[i] = 0;
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
	VectorMap<String, EntrySuggestions> suggestions;
	Vector<ReviewItem> review_queue;
	Index<String> dismissed_review_ids;
	Vector<HistoryEvent> history;
	int max_history = 1000;
	Vector<String> known_current_tags;
	Vector<String> known_reason_tags;
	Vector<String> known_gap_tags;
	
	void Jsonize(JsonIO& jio) {
		jio("version", version)("working_dir", working_dir)("metadata", metadata)
		   ("suggestions", suggestions)
		   ("dismissed_review_ids", dismissed_review_ids)
		   ("history", history)
		   ("known_current_tags", known_current_tags)
		   ("known_reason_tags", known_reason_tags)
		   ("known_gap_tags", known_gap_tags);
	}
	
	void Reset() {
		path = "";
		working_dir = "";
		version = 1;
		metadata.Clear();
		suggestions.Clear();
		review_queue.Clear();
		dismissed_review_ids.Clear();
		history.Clear();
		known_current_tags.Clear();
		known_reason_tags.Clear();
		known_gap_tags.Clear();
	}
	
	FileMetadata GetEffectiveMetadata(const String& rel_path) const;
	String GetBackupPath() const;
	bool WriteBackup() const;

	void AnalyzeEntry(const String& rel_path);
	void RunConsistencyCheck();
	ProjectDashboard GetDashboard() const;

	void LogEvent(const String& path, const String& type, const String& desc, const String& old_val = "", const String& new_val = "", const String& src = "user");
	
	EntryScore ComputeScore(const String& path) const;
	VectorMap<String, EntryScore> GetActionView(int limit = 0) const;
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
	void OnAnalyze();
	void OnRunConsistencyCheck();
	void OnShowDashboard();
	void OnShowReviewQueue();
	void OnShowTimeline();
	void OnShowActionView();
	void OnShowOverviewPreview();

	void SaveLayout();
	void LoadLayout();

	void CheckAutosave();
	void MarkSession(bool active);
	bool CheckRecovery();

	void ApplySuggestion(const String& path, int type, int category, const String& value);
	void DismissSuggestion(const String& path, int type, int category, const String& value);

	void RefreshReviewQueue();
	void RefreshDashboard();
	void RefreshTimeline();
	void RefreshActionView();
	void RefreshOverviewPreview();

	void OnExportOverview();

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
		Callback when_change;

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
		Callback when_change;

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

	struct SuggestionPanel : ParentCtrl {
		typedef SuggestionPanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		String* current_path;
		EntrySuggestions* suggestions;

		SuggestionPanel();
		void Refresh();
		void OnApply();
		void OnDismiss();
	};

	struct DashboardPanel : ParentCtrl {
		typedef DashboardPanel CLASSNAME;
		ArrayCtrl stats;
		DashboardPanel() { Add(stats.SizePos()); stats.AddColumn("Metric"); stats.AddColumn("Value"); }
		void Refresh(const ProjectDashboard& db);
	};

	struct ReviewQueuePanel : ParentCtrl {
		typedef ReviewQueuePanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		ReviewQueuePanel();
		void Refresh(const Vector<ReviewItem>& queue);
		void OnJump();
		void OnDismiss();
	};

	struct TimelinePanel : ParentCtrl {
		typedef TimelinePanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		TimelinePanel();
		void Refresh(const Vector<HistoryEvent>& history);
		void OnJump();
	};

	struct ActionViewPanel : ParentCtrl {
		typedef ActionViewPanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		ActionViewPanel();
		void Refresh(const VectorMap<String, EntryScore>& view);
		void OnJump();
	};

	struct OverviewPreviewPanel : ParentCtrl {
		typedef OverviewPreviewPanel CLASSNAME;
		RichTextView view;
		Button refresh, export_btn;
		Option markdown;
		OverviewerWindow* window;

		OverviewPreviewPanel();
		void Refresh();
	};

	TagPanel current_tags_pane, reason_tags_pane, gap_tags_pane;
	ListPanel problems_pane, tasks_pane, leads_pane;
	SuggestionPanel suggestion_pane;
	DashboardPanel dashboard_pane;
	ReviewQueuePanel review_queue_pane;
	TimelinePanel timeline_pane;
	ActionViewPanel action_view_pane;
	OverviewPreviewPanel overview_preview_pane;

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
	DockableCtrl* dock_suggestions = nullptr;
	DockableCtrl* dock_dashboard = nullptr;
	DockableCtrl* dock_review_queue = nullptr;
	DockableCtrl* dock_timeline = nullptr;
	DockableCtrl* dock_action_view = nullptr;
	DockableCtrl* dock_overview_preview = nullptr;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void ToolsMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	
	void CreateFlagsPane();
	void CreateNumericPane();
	void CreateInfoPane();
	
	FileMetadata dummy_metadata; // for when nothing is selected
	EntrySuggestions dummy_suggestions;
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
