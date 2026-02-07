#ifndef _MaestroHub_MaestroHub_h_
#define _MaestroHub_MaestroHub_h_

#include <CtrlLib/CtrlLib.h>
#include <Maestro/Maestro.h>
#include <GraphLib/GraphLib.h>
#include <CodeEditor/CodeEditor.h>
#include <PdfDraw/PdfDraw.h>

NAMESPACE_UPP

// Define the layout once for the entire package
#define LAYOUTFILE <MaestroHub/MaestroHub.lay>
#include <CtrlCore/lay.h>

// 1. Tool Components
class MaestroAssistant : public WithGlobalAssistantLayout<ParentCtrl> {
public:
	AIChatCtrl chat;
	ArrayCtrl  context_stack;
	Splitter   vsplit;
	bool       is_expanded = true;

	void Toggle();
	void UpdateContext(const String& track, const String& phase, const String& task);
	typedef MaestroAssistant CLASSNAME;
	MaestroAssistant();
};

class TUBrowser : public WithTUBrowserLayout<ParentCtrl> {
public:
	ParentCtrl left_pane;
	TabCtrl    details;
	EditString pkg_search, sym_search;
	ArrayCtrl  pkg_list, sym_list;
	ParentCtrl symbol_pane;
	RichTextView dep_view;
	Splitter   split;

	One<TuManager> tum;
	String root;
	void Load(const String& maestro_root);
		void UpdatePackages();
		void OnPackageCursor();
		void UpdateSymbols();
		void OnSynthesize();
		
		Callback1<String> WhenSynthesize;
		
		typedef TUBrowser CLASSNAME;
		TUBrowser();
	};
	

struct SolutionPattern : Moveable<SolutionPattern> {
	String name;
	String regex;
	String description;
	String prompt_template;
	
	void Jsonize(JsonIO& jio) {
		jio("name", name)("regex", regex)("description", description)("prompt_template", prompt_template);
	}
};

class SolutionsManager {
public:
	Array<SolutionPattern> patterns;
	String root;
	
	void Load(const String& maestro_root);
	void Save();
	int Match(const String& message); // Returns index or -1
};

class LogAnalyzer : public WithLogAnalyzerLayout<ParentCtrl> {
public:
	ArrayCtrl    scan_list, finding_list;
	Splitter     hsplit, vsplit;
	RichTextView detail_view;

	One<LogManager> lm;
	SolutionsManager sm;
	String       root;
	void Load(const String& maestro_root);
	void UpdateScans();
	void OnScanCursor();
	void OnFindingCursor();
	void OnCreateIssue();
	void OnRemediate();
	
	Callback1<String> WhenRemediate;
	
	typedef LogAnalyzer CLASSNAME;
	LogAnalyzer();
};

class ConversionPane : public WithConversionLayout<ParentCtrl> {
public:
	String root;
	WorkGraph plan;
	
	TreeArrayCtrl  transformation_tree;
	TabCtrl        workspace_tabs;
	RichTextView   diff_pane, ai_rationale, progress_log;
	Splitter       center_split;

	void Load(const String& maestro_root);
	void OnInventory();
	void OnPlan();
	void OnRun();
	void OnValidate();
	void OnToolbar(Bar& bar);
	void OnTreeCursor();
	void Log(const String& txt, Color clr = Black());
	
	typedef ConversionPane CLASSNAME;
	ConversionPane();
};

// 2. Hub Panes
class FleetDashboard : public WithFleetDashboardLayout<ParentCtrl> {
public:
	TreeArrayCtrl project_grid;
	ArrayCtrl     automation_queue;
	Splitter      vsplit;

	void LoadProjects(const Vector<String>& paths);
	void UpdateQueue();
	typedef FleetDashboard CLASSNAME;
	FleetDashboard();
};

class IntelligenceHub : public WithIntelligenceHubLayout<ParentCtrl> {
public:
	One<TUBrowser>   tu_browser;
	One<LogAnalyzer> log_analyzer;
	One<ConversionPane> conversion;
	RepoView         repo;
	TabCtrl          tabs;

	void Load(const String& maestro_root);
	typedef IntelligenceHub CLASSNAME;
	IntelligenceHub();
};

class PlaybookVisualLogic : public ParentCtrl {
public:
	CodeEditor puml_editor;
	GraphLib::GraphNodeCtrl graph_view;
	Splitter   split;
	
	void Load(const String& puml);
	void UpdatePreview();
	String Get() const;
	typedef PlaybookVisualLogic CLASSNAME;
	PlaybookVisualLogic();
};

class PlaybookPane : public WithPlaybookLayout<ParentCtrl> {
public:
	String root;
	Array<Playbook> playbooks;
	PlaybookVisualLogic visual_logic;
	
	void Load(const String& maestro_root);
	void OnPlaybookCursor();
	void OnNew();
	void OnSave();
	void OnValidate();
	void OnToolbar(Bar& bar);
	typedef PlaybookPane CLASSNAME;
	PlaybookPane();
};

class EvidencePane : public WithEvidenceLayout<ParentCtrl> {
public:
	ArrayCtrl    evidence_list;
	RichTextView detail_view;
	Splitter     main_split;
	ToolBar      toolbar;

	String       root;
	void Load(const String& maestro_root);
	void OnEvidenceCursor();
	void OnCollect();
	void OnVerify();
	void OnExportPdf();
	void OnToolbar(Bar& bar);
	typedef EvidencePane CLASSNAME;
	EvidencePane();
};

class AuditTrailCorrelator : public WithAuditTrailLayout<ParentCtrl> {
public:
	ArrayCtrl    event_list;
	RichTextView detail_view;
	Splitter     vsplit;

	void Load(const String& maestro_root);
	void OnEventCursor();
	typedef AuditTrailCorrelator CLASSNAME;
	AuditTrailCorrelator();
};

class DebugWorkspace : public WithDebugWorkspaceLayout<ParentCtrl> {
public:
	ParentCtrl left_pane, center_pane, bottom_pane;
	DropList   target_device;
	TreeCtrl   call_stack;
	RichTextCtrl source_code;
	ArrayCtrl  locals;
	Splitter   hsplit, vsplit;
	ToolBar    toolbar;
	
	One<DebuggerService> dbg;
	Event<String> WhenLog;

	void Load(const String& maestro_root);
	void OnRun();
	void OnStop();
	void OnStep();
	void OnToolbar(Bar& bar);
	typedef DebugWorkspace CLASSNAME;
	DebugWorkspace();
};

class TechnologyPane : public ParentCtrl {
public:
	Splitter split;
	RepoView repo;
	PlanView plan;
	String   root;
	Callback3<String, String, String> WhenEnact;
	void Load(const String& root);
	typedef TechnologyPane CLASSNAME;
	TechnologyPane();
};

class PipelinePane : public ParentCtrl {
public:
	typedef PipelinePane CLASSNAME;
	PipelinePane();
	void Load(const String& root);

	WithPipelineLayout<ParentCtrl> layout;
};

class ProductPane : public ParentCtrl {
public:
	Splitter split, vsplit_rb, vsplit_wg, wg_split;
	ArrayCtrl runbooks, workflows;
	RichTextView rb_detail, wg_detail;
	GraphLib::GraphNodeCtrl workflow_graph;
	ParentCtrl workflow_view;
	
	Array<Runbook> runbook_data;
	Vector<WorkGraph> workflow_data;
	
	String      root;
	void Load(const String& root);
	void OnEnactStep(int step, const String& instruction);
	void OnRunbookSelect();
	void OnWorkflowSelect();
	void OnNodeClick(GraphLib::Node& n);
	void OnNodeRightClick(GraphLib::Node& n);
	
	Callback3<String, int, String> WhenEnactStep;

	typedef ProductPane CLASSNAME;
	ProductPane();
};

class UXEvaluationFactory : public TopWindow {
public:
	Splitter   split;
	ArrayCtrl  test_list;
	
	ImageCtrl  baseline_view;
	ImageCtrl  current_view;
	ImageCtrl  diff_view;
	ParentCtrl images;
	
	Button     run_test, approve;
	
	String     root;

	void Load(const String& maestro_root);
	void OnRunTest();
	void OnApprove();
	void OnTestCursor();
	
	typedef UXEvaluationFactory CLASSNAME;
	UXEvaluationFactory();
};

class MaintenancePane : public WithMaintenanceLayout<ParentCtrl> {
public:
	Splitter split;
	ArrayCtrl cache_list;
	RichTextView cache_detail;

	String root;
	void Load(const String& root);
	void OnPurgeCache();
	void OnPurgeTracks();
	void OnSyncCore();
	void OnCacheCursor();
	
	typedef MaintenancePane CLASSNAME;
	MaintenancePane();
};

class IssuesPane : public ParentCtrl {
public:
	ArrayCtrl    issues;
	RichTextView detail;
	Splitter     main_split;
	String       current_root;
	void Load(const String& root);
	void OnMenu(Bar& bar);
	void OnTriage();
	void OnResolve();
	void OnEdit();
	void OnBulkStatus();
	void OnBulkSeverity();
	typedef IssuesPane CLASSNAME;
	IssuesPane();
};

class WorkPane : public WithWorkDashboardLayout<ParentCtrl> {
public:
	String active_session_id;
	String current_root;
	void OnSubwork();
	void Load(const String& root);
	void Refresh();
	void OnApprove();
	void OnReject();
	typedef WorkPane CLASSNAME;
	WorkPane();
};

class SessionManagementPane : public WithSessionManagementLayout<ParentCtrl> {
public:
	Splitter split, detail_split;
	ParentCtrl left_pane, work_pane;
	ArrayCtrl dirs, work_sessions, breadcrumbs;
	SessionListView sessions;
	TabCtrl list_tabs, detail_tabs;
	ParentCtrl filter_bar;
	DropList filter_type, filter_status;
	EditString filter_search;
	RichTextView context_view;

	String current_root;
	CliMaestroEngine engine;
	Callback2<String, String> WhenSelect;
	void Load(const String& root);
	void OnDirCursor();
	void UpdateWorkSessionList();
	void OnWorkSessionCursor();
	void OnSessionMenu(Bar& bar);
	void DeleteSession();
	typedef SessionManagementPane CLASSNAME;
	SessionManagementPane();
};

class TutorialPane : public ParentCtrl {
public:
	RichTextView view;
	int step = 0;
	
	void UpdateContent();
	void OnNext();
	void OnNextStep();
	void OnPrev();
	typedef TutorialPane CLASSNAME;
	TutorialPane();
};

// 3. Dialogs
class IssueEditDialog : public WithIssueEditLayout<TopWindow> {
public:
	void SyncFromIssue(const MaestroIssue& src);
	void SyncToIssue(MaestroIssue& dest) const;
	typedef IssueEditDialog CLASSNAME;
	IssueEditDialog();
};

class IssueCreateDialog : public WithIssueCreateLayout<TopWindow> {
public:
	MaestroIssue GetIssue() const;
	typedef IssueCreateDialog CLASSNAME;
	IssueCreateDialog();
};

class ListSelectDialog : public WithListSelectLayout<TopWindow> {
public:
	void SetChoices(const Vector<String>& choices);
	bool RunSelect(const char *title, const char *label, String& result);
	typedef ListSelectDialog CLASSNAME;
	ListSelectDialog();
};

class PlaybookSelectDialog : public WithListSelectLayout<TopWindow> {
public:
	ArrayCtrl list;
	String selected_id;
	void Load(const String& maestro_root);
	typedef PlaybookSelectDialog CLASSNAME;
	PlaybookSelectDialog();
};

class TriageDialog : public WithTriageLayout<TopWindow> {
public:
	One<IssueManager> ism;
	Array<MaestroIssue> pending;
	int cursor = 0;
	String root;

	void Load(const String& maestro_root);
	void UpdateUI();
	void Advance();
	String FormatIssueInfo(const MaestroIssue& iss) const;
	String FormatAiSuggestion(const MaestroIssue& iss) const;
	void OnAccept();
	void OnSkip();
	void OnIgnore();
	void OnEdit();
	void OnCreateTask();
	typedef TriageDialog CLASSNAME;
	TriageDialog();
};

class RunbookEditor : public WithRunbookEditorLayout<TopWindow> {
public:
	Button ok, cancel;
	Callback1<String> WhenAssist;
	
	void New(const String& root);
	void OnEditStep();
	
	typedef RunbookEditor CLASSNAME;
	RunbookEditor();
};

class StateEditor : public WithStateEditorLayout<TopWindow> {
public:
	One<WorkflowManager> wfm;
	String root, current_id;
	void OnToolbar(Bar& bar);
	void Load(const String& maestro_root, const String& id);
	void UpdatePreview();
	void NewState();
	void NewTransition();
	void Save();
	typedef StateEditor CLASSNAME;
	StateEditor();
};

class StepWizard : public WithStepWizardLayout<TopWindow> {
public:
	Button ai_assist;
	Callback1<String> WhenAssist;

	RunbookStep step;
	void SetStep(const RunbookStep& s);
	RunbookStep GetStep();
	void AddVariant();
	void RemoveVariant();
	typedef StepWizard CLASSNAME;
	StepWizard();
};

class SubworkManagerDialog : public WithSubworkLayout<TopWindow> {
public:
	TreeArrayCtrl subwork_tree;
	ArrayCtrl     context_stack;
	Button        btn_push, btn_pop, btn_close;

	String root, active_session_id;
	void Load(const String& root, const String& session_id);
	void UpdateUI();
	void OnPush();
	void OnPop();
	typedef SubworkManagerDialog CLASSNAME;
	SubworkManagerDialog();
};

class NewSessionDialog : public WithNewSessionLayout<TopWindow> {
public:
	String session_id;
	void OnOK();
	typedef NewSessionDialog CLASSNAME;
	NewSessionDialog();
};

class InitDialog : public WithInitLayout<TopWindow> {
public:
	void OnBrowse();
	typedef InitDialog CLASSNAME;
	InitDialog();
};

class ConfigurationDialog : public WithConfigurationLayout<TopWindow> {
public:
	One<SettingsManager> sm;
	String root;
	void Load(const String& root);
	void Save();
	typedef ConfigurationDialog CLASSNAME;
	ConfigurationDialog();
};

class OpsRunner : public WithOpsRunnerLayout<TopWindow> {
public:
	String root;
	void Load(const String& root);
	void OnRun();
	typedef OpsRunner CLASSNAME;
	OpsRunner();
};

class WelcomeDialog : public WithWelcomeLayout<TopWindow> {
public:
	typedef WelcomeDialog CLASSNAME;
	WelcomeDialog();
};

struct Toolchain : Moveable<Toolchain> {
	String name;
	String path;
	VectorMap<String, String> vars;
	
	String ToString() const { return name; }
};

class ToolchainManager {
public:
	Array<Toolchain> toolchains;
	
	void Load(const String& dir);
	void ScanStandardLocations();
	Toolchain* Find(const String& name);
};

class BuildMethodsDialog : public TopWindow {
public:
	Splitter split;
	ArrayCtrl list;
	ArrayCtrl vars;
	
	ToolchainManager tm;
	
	void Load();
	void OnSelect();
	
	typedef BuildMethodsDialog CLASSNAME;
	BuildMethodsDialog();
};

class SolutionsHub : public TopWindow {
public:
	Splitter split;
	ArrayCtrl list;
	ParentCtrl editor;
	EditString name, regex;
	EditString description;
	RichTextCtrl prompt_template;
	Button add, remove, save;
	
	SolutionsManager sm;
	String root;
	
	void Load(const String& maestro_root);
	void OnSelect();
	void OnAdd();
	void OnRemove();
	void OnSave();
	void OnTest();
	
	typedef SolutionsHub CLASSNAME;
	SolutionsHub();
};

// 4. Main Window
bool CreateIssueTaskFile(const String& root, const MaestroIssue& iss, const String& title, String& task_path);

class MaestroHubCockpit : public TopWindow {
public:
	MenuBar   menu;
	ToolBar   toolbar;
	StatusBar statusbar;
	Splitter  main_split, center_split;
	TabCtrl   left_tabs, center_tabs, bottom_tabs;
	
	One<MaestroAssistant>      assistant;
	One<FleetDashboard>        fleet;
	One<IntelligenceHub>       intelligence;
	One<EvidencePane>          evidence;
	One<PlaybookPane>          playbook;
	One<AuditTrailCorrelator>  audit_trail;
	One<DebugWorkspace>        debug_workspace;
	One<TechnologyPane>        technology;
	One<PipelinePane>          pipeline;
	One<ProductPane>           product;
	One<MaintenancePane>       maintenance;
	One<IssuesPane>            issues;
	One<WorkPane>              work;
	One<SessionManagementPane> sessions;
	One<TutorialPane>          tutorial;
	
	RichTextView               automation_output;
	RichTextView               ai_trace;
	RichTextView               internal_console;
	ProgressIndicator          quota_indicator;
	
	void LogInternal(const String& msg, int level = 0); // 0:Info, 1:Warn, 2:Error
	void UpdateQuota(double percent);
	
	RecentConfig config;
	String       current_root;
	String       active_track, active_phase, active_task;
	
	void MainMenu(Bar& bar);
	void AppMenu(Bar& bar);
	void SelectRoot();
	void OnToggleAssistant();
	void OnAssistantEvent(const MaestroEvent& e);
	void OnEnact(String track, String phase, String task);
	void OnEnactStep(String runbook_title, int step_n, String instruction);
	void OnSessionSelect(String backend, String session_id);
	void OnNewSession();
	void OnCreateIssue();
	void OnInitMaestro();
	void OnTriageWizard();
	void OnRunbookEditor();
	void OnStateEditor();
	void OnTUBrowser();
	void OnLogAnalyzer();
	void OnSettings();
	void OnBuildMethods();
	void OnOpsRunner();
	void OnSuggestEnact();
	
	void SyncStatus();
	void LoadData();
	void PlanWatcher();
	void ScanForUnblockedTasks();
	Time last_plan_check;
	
	// Navigation History
	Vector<int> tab_history;
	int         history_pos = -1;
	bool        navigating = false;
	
	void NavigateTo(int tab);
	void OnBack();
	void OnNext();

public:
	typedef MaestroHubCockpit CLASSNAME;
	MaestroHubCockpit();
	~MaestroHubCockpit();
};

END_UPP_NAMESPACE

#endif
