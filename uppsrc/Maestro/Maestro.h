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

}

#endif
