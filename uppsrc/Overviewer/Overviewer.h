#ifndef _Overviewer_Overviewer_h_
#define _Overviewer_Overviewer_h_

#include <CtrlLib/CtrlLib.h>
#include <Docking/Docking.h>
#include "Settings.h"
#include "OverviewGenerator.h"
#include "GitContext.h"
#include "InsightEngine.h"

using namespace Upp;

// Standard layout inclusion pattern
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
	
	EntrySuggestions() {}
	EntrySuggestions(const EntrySuggestions& s) { *this = s; }
	EntrySuggestions(const EntrySuggestions& s, int) { *this = s; }
	EntrySuggestions& operator=(const EntrySuggestions& s) {
		current_tags <<= s.current_tags; reason_tags <<= s.reason_tags; gap_tags <<= s.gap_tags;
		problems <<= s.problems; tasks <<= s.tasks;
		return *this;
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
	String actor_id;
	String actor_type;
	String session_id;
	String scenario_id;

	void Jsonize(JsonIO& jio) {
		jio("time", time)("path", path)("type", type)("description", description)
		   ("old_value", old_value)("new_value", new_value)("source", source)
		   ("actor_id", actor_id)("actor_type", actor_type)("session_id", session_id)
		   ("scenario_id", scenario_id);
	}
};

struct SessionInfo : Moveable<SessionInfo> {
	String session_id;
	Time start_time;
	String actor_id;
	String actor_type;

	void Jsonize(JsonIO& jio) {
		jio("session_id", session_id)("start_time", start_time)("actor_id", actor_id)("actor_type", actor_type);
	}
};

struct EntryScore : Moveable<EntryScore> {
	double score = 0;
	Vector<String> factors;

	void Jsonize(JsonIO& jio) {
		jio("score", score)("factors", factors);
	}
	
	EntryScore() {}
	EntryScore(const EntryScore& s) { *this = s; }
	EntryScore(const EntryScore& s, int) { *this = s; }
	EntryScore& operator=(const EntryScore& s) {
		score = s.score;
		factors <<= s.factors;
		return *this;
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
	
	FileMetadata() {}
	FileMetadata(const FileMetadata& s) { *this = s; }
	FileMetadata(const FileMetadata& s, int) { *this = s; }
	FileMetadata& operator=(const FileMetadata& s) {
		flags = s.flags; quality = s.quality; completion = s.completion; priority = s.priority;
		notes = s.notes;
		current_tags <<= s.current_tags; reason_tags <<= s.reason_tags; gap_tags <<= s.gap_tags;
		problems <<= s.problems; tasks <<= s.tasks; leads <<= s.leads;
		return *this;
	}
};

struct Scenario : Moveable<Scenario> {
	String id;
	String name;
	VectorMap<String, FileMetadata> metadata_delta;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("name", name)("metadata_delta", metadata_delta);
	}
	
	Scenario() {}
	Scenario(const Scenario& s) { *this = s; }
	Scenario(const Scenario& s, int) { *this = s; }
	Scenario& operator=(const Scenario& s) {
		id = s.id; name = s.name;
		metadata_delta.Clear();
		for(int i = 0; i < s.metadata_delta.GetCount(); i++)
			metadata_delta.Add(s.metadata_delta.GetKey(i), s.metadata_delta[i]);
		return *this;
	}
};

struct Decision : Moveable<Decision> {
	String id;
	String title;
	String description;
	Time timestamp;
	String actor_id;
	String actor_type;
	String session_id;
	Vector<String> related_entries;
	String related_scenario_id;
	String status; // proposed, accepted, rejected, superseded
	Vector<String> tags;
	Vector<String> linked_commits;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("title", title)("description", description)("timestamp", timestamp)
		   ("actor_id", actor_id)("actor_type", actor_type)("session_id", session_id)
		   ("related_entries", related_entries)("related_scenario_id", related_scenario_id)
		   ("status", status)("tags", tags)("linked_commits", linked_commits);
	}
	
	Decision() {}
	Decision(const Decision& s) { *this = s; }
	Decision(const Decision& s, int) { *this = s; }
	Decision& operator=(const Decision& s) {
		id = s.id; title = s.title; description = s.description; timestamp = s.timestamp;
		actor_id = s.actor_id; actor_type = s.actor_type; session_id = s.session_id;
		related_entries <<= s.related_entries; related_scenario_id = s.related_scenario_id;
		status = s.status; tags <<= s.tags; linked_commits <<= s.linked_commits;
		return *this;
	}
};

struct Comment : Moveable<Comment> {
	String id;
	String text;
	Time timestamp;
	String actor_id;
	String related_entry;
	String related_decision;

	void Jsonize(JsonIO& jio) {
		jio("id", id)("text", text)("timestamp", timestamp)("actor_id", actor_id)
		   ("related_entry", related_entry)("related_decision", related_decision);
	}
};

struct UndoEvent : Moveable<UndoEvent> {
	String path;
	FileMetadata old_meta;
	FileMetadata new_meta;
	
	UndoEvent() {}
	UndoEvent(const UndoEvent& s) { *this = s; }
	UndoEvent(const UndoEvent& s, int) { *this = s; }
	UndoEvent& operator=(const UndoEvent& s) {
		path = s.path;
		old_meta = s.old_meta;
		new_meta = s.new_meta;
		return *this;
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
	VectorMap<String, int> activity_by_actor;
	
	int proposed_decisions = 0;
	int accepted_decisions = 0;
	int total_comments = 0;
	int active_insights = 0;

	ProjectDashboard() {
		total_files = total_dirs = flagged_entries = needs_review = 0;
		missing_priority = missing_completion = with_notes = with_problems = 0;
		with_tasks = with_leads = suggestions_pending = 0;
		recent_changes = stale_entries = 0;
		proposed_decisions = accepted_decisions = total_comments = active_insights = 0;
		for(int i = 0; i < 6; i++) priority_counts[i] = 0;
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
	Vector<SessionInfo> sessions;
	int max_history = 1000;
	int max_sessions = 100;
	Vector<String> known_current_tags;
	Vector<String> known_reason_tags;
	Vector<String> known_gap_tags;
	
	GitContext git;
	
	String current_actor_id = "user";
	String current_actor_type = "user";
	String current_session_id;
	
	VectorMap<String, Scenario> scenarios;
	String active_scenario_id;
	
	VectorMap<String, Decision> decisions;
	Vector<Comment> comments;
	Vector<Insight> insights;

	void Jsonize(JsonIO& jio) {
		jio("version", version)("working_dir", working_dir)("metadata", metadata)
		   ("suggestions", suggestions)
		   ("dismissed_review_ids", dismissed_review_ids)
		   ("history", history)
		   ("sessions", sessions)
		   ("known_current_tags", known_current_tags)
		   ("known_reason_tags", known_reason_tags)
		   ("known_gap_tags", known_gap_tags)
		   ("scenarios", scenarios)
		   ("decisions", decisions)
		   ("comments", comments)
		   ("insights", insights);
	}
	
	void Reset() {
		path = ""; working_dir = ""; version = 1;
		metadata.Clear(); suggestions.Clear(); review_queue.Clear();
		dismissed_review_ids.Clear(); history.Clear(); sessions.Clear();
		known_current_tags.Clear(); known_reason_tags.Clear(); known_gap_tags.Clear();
		scenarios.Clear(); active_scenario_id = "";
		decisions.Clear(); comments.Clear(); insights.Clear();
	}
	
	FileMetadata GetEffectiveMetadata(const String& rel_path) const;
	FileMetadata& GetMetadataWrite(const String& rel_path);
	String GetBackupPath() const;
	bool WriteBackup() const;

	void AnalyzeEntry(const String& rel_path);
	void RunConsistencyCheck();
	ProjectDashboard GetDashboard() const;

	void LogEvent(const String& path, const String& type, const String& desc, const String& old_val = "", const String& new_val = "", const String& src = "user");
	void StartSession(const String& actor_id, const String& actor_type);

	EntryScore ComputeScore(const String& path) const;
	VectorMap<String, EntryScore> GetActionView(int limit = 0) const;
	
	void RefreshGit();
	
	String CreateScenario(const String& name);
	void ActivateScenario(const String& id);
	void DeactivateScenario();
	void ApplyScenario(const String& id);
	
	String CreateDecision(const String& title);
	void UpdateDecision(const String& id, const String& desc, const String& status);
	void LinkDecisionToEntry(const String& id, const String& path);
	void LinkDecisionToScenario(const String& id, const String& scenario_id);
	
	String AddComment(const String& text, const String& entry = "", const String& decision = "");
	
	void GenerateInsights();
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

	void MarkDirty() { dirty = true; SyncTitle(); SyncStatusBar(); }
	void ClearDirty() { dirty = false; SyncTitle(); SyncStatusBar(); }
	bool IsDirty() const { return dirty; }

	void SyncTitle();
	void SyncStatusBar();
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
	void OnShowGitHistory();
	void OnShowSessions();
	void OnShowDecisions();
	void OnShowComments();
	void OnShowInsights();
	
	void OnScenarioMenu(Bar& bar);
	void OnCreateScenario();
	void OnActivateScenario(String id);
	void OnDeactivateScenario();
	void OnApplyScenario();
	void OnCompareScenario();

	void SaveLayout();
	void LoadLayout();
	void ResetLayout();

	void CheckAutosave();
	void MarkSession(bool active);
	bool CheckRecovery();
	void CheckExternalChange();

	void ApplySuggestion(const String& path, int type, int category, const String& value);
	void DismissSuggestion(const String& path, int type, int category, const String& value);

	void RefreshReviewQueue();
	void RefreshDashboard();
	void RefreshTimeline();
	void RefreshActionView();
	void RefreshOverviewPreview();
	void RefreshGitHistory();
	void RefreshSessions();
	void RefreshDecisions();
	void RefreshComments();
	void RefreshInsights();

	void OnExportOverview();
	void OnRefreshGit();
	void OnAddComment();
	void OnDismissInsight();
	void OnJumpInsight();
	
	void Undo();
	void Redo();
	void RecordUndo(const String& path, const FileMetadata& old_m, const FileMetadata& new_m);

	void OnSearch();

public:
	struct FilterConfig {
		int mode = 0; // 0: all, 1: advanced
		uint32 flags = 0;
		int priority_min = 0;
		bool missing_priority = false;
		bool missing_completion = false;
		String tag_current, tag_reason, tag_gap;
		String search_text;
	} filter;

private:
	OverviewerProject project;
	bool dirty = false;
	String current_selection;
	Time last_autosave;
	Time last_file_time;

	MenuBar menu;
	ToolBar quick_actions;
	StatusBar status_bar;
	
	EditString search_ctrl;
	
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

		TagPanel() : assigned(nullptr), global_known(nullptr) {
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

		ListPanel() : items(nullptr) {
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

	struct GitHistoryPanel : ParentCtrl {
		typedef GitHistoryPanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		GitHistoryPanel();
		void Refresh(const Vector<GitCommit>& history);
		void OnLink();
	};

	struct SessionPanel : ParentCtrl {
		typedef SessionPanel CLASSNAME;
		ArrayCtrl list;
		SessionPanel() { Add(list.SizePos()); list.AddColumn("ID"); list.AddColumn("Start"); list.AddColumn("Actor"); }
		void Refresh(const Vector<SessionInfo>& sessions);
	};

	struct ScenarioDiffPanel : TopWindow {
		typedef ScenarioDiffPanel CLASSNAME;
		ArrayCtrl list;
		ScenarioDiffPanel() {
			Title("Scenario Diff");
			SetRect(0, 0, 400, 300);
			Add(list.SizePos());
			list.AddColumn("Path");
			list.AddColumn("Change");
		}
		void Refresh(const String& scenario_name, const VectorMap<String, String>& diff) {
			list.Clear();
			Title("Scenario Diff: " + scenario_name);
			for(int i = 0; i < diff.GetCount(); i++)
				list.Add(diff.GetKey(i), diff[i]);
		}
	};

	struct DecisionPanel : ParentCtrl {
		typedef DecisionPanel CLASSNAME;
		ArrayCtrl list;
		DocEdit description;
		Button add, accept, reject;
		OverviewerWindow* window;
		
		DecisionPanel();
		void Refresh(const VectorMap<String, Decision>& decisions);
		void OnAdd();
		void OnStatus(String status);
		void OnSel();
		void OnDescChange();
	};

	struct CommentPanel : ParentCtrl {
		typedef CommentPanel CLASSNAME;
		ArrayCtrl list;
		OverviewerWindow* window;
		CommentPanel() { Add(list.SizePos()); list.AddColumn("Actor", 20); list.AddColumn("Date", 30); list.AddColumn("Text"); }
		void Refresh(const Vector<Comment>& comments, const String& rel_path = "", const String& decision_id = "");
	};

	struct InsightPanel : ParentCtrl {
		typedef InsightPanel CLASSNAME;
		ArrayCtrl list;
		DocEdit description;
		Button dismiss, jump;
		OverviewerWindow* window;
		
		InsightPanel();
		void Refresh(const Vector<Insight>& insights);
		void OnSel();
	};

	TagPanel current_tags_pane, reason_tags_pane, gap_tags_pane;
	ListPanel problems_pane, tasks_pane, leads_pane;
	SuggestionPanel suggestion_pane;
	DashboardPanel dashboard_pane;
	ReviewQueuePanel review_queue_pane;
	TimelinePanel timeline_pane;
	ActionViewPanel action_view_pane;
	OverviewPreviewPanel overview_preview_pane;
	GitHistoryPanel git_history_pane;
	SessionPanel session_pane;
	ScenarioDiffPanel scenario_diff_pane;
	DecisionPanel decision_pane;
	CommentPanel comment_pane;
	InsightPanel insight_pane;

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
	DockableCtrl* dock_git_history = nullptr;
	DockableCtrl* dock_sessions = nullptr;
	DockableCtrl* dock_decisions = nullptr;
	DockableCtrl* dock_comments = nullptr;
	DockableCtrl* dock_insights = nullptr;

	void MainMenu(Bar& bar);
	void FileMenu(Bar& bar);
	void EditMenu(Bar& bar);
	void ViewMenu(Bar& bar);
	void ToolsMenu(Bar& bar);
	void HelpMenu(Bar& bar);
	
	void QuickActions(Bar& bar);
	
	void CreateFlagsPane();
	void CreateNumericPane();
	void CreateInfoPane();
	
	FileMetadata dummy_metadata; // for when nothing is selected
	EntrySuggestions dummy_suggestions;
	
	Vector<UndoEvent> undo_stack;
	Vector<UndoEvent> redo_stack;
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
