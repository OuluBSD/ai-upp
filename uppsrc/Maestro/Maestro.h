#ifndef MAESTRO_MAESTRO_H
#define MAESTRO_MAESTRO_H

#include <Core/Core.h>
#include <plugin/pcre/Pcre.h>

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#endif

#include <Core/VfsBase/Automation.h>

#ifdef _GraphLib_GraphLib_h_
 #error Wrong inclusion order
#endif

NAMESPACE_UPP

struct Command {
	virtual ~Command() {}
	virtual String GetName() const = 0;
	virtual Vector<String> GetAliases() const { return {}; }
	virtual String GetDescription() const = 0;
	virtual void ShowHelp() const = 0;
	virtual void Execute(const Vector<String>& args) = 0;
};

// 1. Core Data Models
#include "AssemblyInfo.h"
#include "PlanModels.h"
#include "UppParser.h"
#include "RepoScanner.h"
#include "PlanParser.h"

// 2. AI Engine Base
#include "Engine.h"
#include "CliEngine.h"
#include "Engines.h"
#include "PlanSummarizer.h"

// 3. Logic & Tools
#include "WorkSession.h"
#include "Breadcrumb.h"
#include "WorkGraphGenerator.h"
#include "WorkGraphRunner.h"
#include "WorkGraphScorer.h"
#include "StructureTools.h"
#include "VarFileParser.h"
#include "DependencyResolver.h"
#include "InventoryGenerator.h"
#include "ConversionMemory.h"
#include "ConversionPlanner.h"
#include "Playbook.h"
#include "EvidencePack.h"
#include "SemanticIntegrity.h"
#include "PipelineRuntime.h"
#include "RegressionReplay.h"
#include "Debugger.h"

#include "DiscussCommand.h"
#include "ConversionOrchestrator.h"
#include "Plugin.h"

#ifdef flagGUI
// 4. UI
#include "RepoView.h"
#include "PlanView.h"
#include "SessionSelectWindow.h"
#include "AIChatCtrl.h"
#endif

class RunbookManager {
	String base_path;
	String items_dir;
	String index_path;

public:
	RunbookManager(const String& maestro_root = ".");
	
	Array<Runbook> ListRunbooks();
	Runbook        LoadRunbook(const String& id);
	bool           SaveRunbook(const Runbook& rb);
	bool           DeleteRunbook(const String& id);
	
	Runbook        Resolve(const String& text, bool use_ai = true);
	
private:
	void           UpdateIndex(const Runbook& rb);
	void           RebuildIndex();
};

class WorkflowManager {
	String base_path;
	String workflows_dir;

public:
	WorkflowManager(const String& maestro_root = ".");
	
	Vector<String> ListWorkflows();
	String         LoadWorkflow(const String& name);
	bool           SaveWorkflow(const String& name, const String& content);
	bool           DeleteWorkflow(const String& name);
	String         Visualize(const String& name, const String& format = "plantuml");
};

class LogTail {
	String path;
	int64  last_pos = 0;
	bool   active = false;

public:
	Event<String> WhenLine;
	
	void Open(const String& p);
	void Close() { active = false; }
	void Poll();
	bool IsActive() const { return active; }
	
	LogTail() {}
};

class LogManager {
	String base_path;
	String scans_dir;

public:
	LogManager(const String& maestro_root = ".");
	
	String CreateScan(const String& source_path, const String& log_text, const String& kind = "any");
	LogScan LoadScan(const String& scan_id);
	Array<LogScanMeta> ListScans();
	Array<LogFinding> ExtractFindings(const String& log_text, const String& kind_filter);
	
private:
	String GenerateFingerprint(const String& message, const String& tool, const String& file);
};

class SettingsManager {
	String base_path;
	String config_path;

public:
	SettingsManager(const String& maestro_root = ".");
	
	ValueMap LoadSettings();
	bool     SaveSettings(const ValueMap& settings);
	Value    GetSetting(const String& key);
	bool     SetSetting(const String& key, const Value& value);
};

class IssueManager {
	String base_path;
	String docs_issues_dir; // Legacy markdown storage
	String json_issues_dir; // Modern JSON storage

public:
	IssueManager(const String& maestro_root = ".");
	
	Array<MaestroIssue> ListIssues(const String& type = "", const String& severity = "", const String& status = "");
	MaestroIssue        LoadIssue(const String& id);
	bool                SaveIssue(const MaestroIssue& issue);
	bool                DeleteIssue(const String& id);
	
	String              CreateFromLogFinding(const ValueMap& finding, const String& scan_id);
	bool                Triage(const String& id, bool auto_mode = true);
};

class TuManager {
	String base_path;
	String cache_dir;

public:
	TuManager(const String& maestro_root = ".");
	
	Vector<String> ListPackages();
	void Build(const String& package);
	void Info(const String& package);
	void Query(const String& query);
};

class WorkManager {
	String base_path;

public:
	WorkManager(const String& maestro_root = ".");
	
	// Work Item Selection
	struct WorkItem : Moveable<WorkItem> {
		String id;
		String type; // "track", "phase", "issue", "task"
		String name;
		String status;
		String description;
		String reason; // Populated by AI
		double confidence = 0.0;
		
		void Jsonize(JsonIO& jio) {
			jio("id", id)("type", type)("name", name)("status", status)
			   ("description", description)("reason", reason)("confidence", confidence);
		}
	};
	
	Array<WorkItem> LoadAvailableWork();
	WorkItem        SelectBestWorkItem(const Array<WorkItem>& items);
	Array<WorkItem> SelectTopWorkItems(const Array<WorkItem>& items, int count = 3);
	
	// Execution
	bool StartWorkSession(const WorkItem& item);
	bool AnalyzeTarget(const String& target, bool simulate = false);
	bool FixTarget(const String& target, const String& issue_id = "", bool simulate = false);
};

// Command Definitions
struct InitCommand : Command {
	String GetName() const override { return "init"; }
	String GetDescription() const override { return "Initialize Maestro in a repository"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct RunbookCommand : Command {
	String GetName() const override { return "runbook"; }
	Vector<String> GetAliases() const override { return {"runba", "rb"}; }
	String GetDescription() const override { return "Runbook-first bootstrap before workflow"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct WorkflowCommand : Command {
	String GetName() const override { return "workflow"; }
	String GetDescription() const override { return "UML-like programming and statemachine management"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct LogCommand : Command {
	String GetName() const override { return "log"; }
	String GetDescription() const override { return "Log scanning and observability commands"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct CacheCommand : Command {
	String GetName() const override { return "cache"; }
	String GetDescription() const override { return "Manage AI cache"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct TrackCacheCommand : Command {
	String GetName() const override { return "track-cache"; }
	String GetDescription() const override { return "Manage the persistent track/phase/task cache"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct OpsCommand : Command {
	String GetName() const override { return "ops"; }
	String GetDescription() const override { return "Operations automation (health checks, runbook execution)"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct PhaseCommand : Command {
	String GetName() const override { return "phase"; }
	Vector<String> GetAliases() const override { return {"ph", "p"}; }
	String GetDescription() const override { return "Manage project phases"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct TaskCommand : Command {
	String GetName() const override { return "task"; }
	Vector<String> GetAliases() const override { return {"ta"}; }
	String GetDescription() const override { return "Manage project tasks"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct SettingsCommand : Command {
	String GetName() const override { return "settings"; }
	Vector<String> GetAliases() const override { return {"config", "cfg"}; }
	String GetDescription() const override { return "Manage project configuration"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct IssuesCommand : Command {
	String GetName() const override { return "issues"; }
	String GetDescription() const override { return "Issue tracking commands"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct SolutionsCommand : Command {
	String GetName() const override { return "solutions"; }
	String GetDescription() const override { return "Solution management commands"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct AiCommand : Command {
	String GetName() const override { return "ai"; }
	String GetDescription() const override { return "AI workflow helpers"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct WorkCommand : Command {
	String GetName() const override { return "work"; }
	Vector<String> GetAliases() const override { return {"wk"}; }
	String GetDescription() const override { return "AI work sessions"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct TuCommand : Command {
	String GetName() const override { return "tu"; }
	String GetDescription() const override { return "Translation unit analysis and indexing"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct UxCommand : Command {
	String GetName() const override { return "ux"; }
	String GetDescription() const override { return "UX evaluation and telemetry tools"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct TutorialCommand : Command {
	String GetName() const override { return "tutorial"; }
	Vector<String> GetAliases() const override { return {"tut"}; }
	String GetDescription() const override { return "Interactive tutorials for Maestro features"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

#ifdef flagGUI
struct TestCommand : Command {
	String GetName() const override { return "test"; }
	Vector<String> GetAliases() const override { return {"tst"}; }
	String GetDescription() const override { return "Execute UI automation tests (Python)"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};
#endif

struct PlanCommand : Command {
	String GetName() const override { return "plan"; }
	Vector<String> GetAliases() const override { return {"pl"}; }
	String GetDescription() const override { return "Plan management"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct RepoCommand : Command {
	String GetName() const override { return "repo"; }
	Vector<String> GetAliases() const override { return {"r"}; }
	String GetDescription() const override { return "Repository analysis and resolution commands"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct MakeCommand : Command {
	String GetName() const override { return "make"; }
	Vector<String> GetAliases() const override { return {"b"}; }
	String GetDescription() const override { return "Build, clean, and manage packages"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct TrackCommand : Command {
	String GetName() const override { return "track"; }
	Vector<String> GetAliases() const override { return {"tr", "t"}; }
	String GetDescription() const override { return "Manage project tracks"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

struct WSessionCommand : Command {
	String GetName() const override { return "wsession"; }
	Vector<String> GetAliases() const override { return {"ws", "s"}; }
	String GetDescription() const override { return "Work session management"; }
	void ShowHelp() const override;
	void Execute(const Vector<String>& args) override;
};

// 5. Global Tool Registration
void RegisterMaestroTools(MaestroToolRegistry& reg);
void RegisterUxTools(MaestroToolRegistry& reg);
Value MaestroUpdateTaskStatus(const ValueMap& params);

String FindPlanRoot();
String GetDocsRoot(const String& plan_root);

END_UPP_NAMESPACE

#endif