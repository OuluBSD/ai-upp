#ifndef MAESTRO_MAESTRO_H
#define MAESTRO_MAESTRO_H

#include <Core/Core.h>
#include <plugin/pcre/Pcre.h>

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#endif

namespace Upp {

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

#include "DiscussCommand.h"
#include "ConversionOrchestrator.h"

#ifdef flagGUI
// 4. UI
#include "RepoView.h"
#include "PlanView.h"
#include "SessionSelectWindow.h"
#include "AIChatCtrl.h"
#endif

// 5. Global Tool Registration
void RegisterMaestroTools(MaestroToolRegistry& reg);
Value MaestroUpdateTaskStatus(const ValueMap& params);

String FindPlanRoot();
String GetDocsRoot(const String& plan_root);

}

#endif
