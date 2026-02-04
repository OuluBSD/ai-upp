#ifndef MAESTRO_MAESTRO_H
#define MAESTRO_MAESTRO_H

// NOTE: This header is normally included inside namespace Upp

#ifndef _Maestro_Maestro_h_
#define _Maestro_Maestro_h_

#ifdef flagGUI
#include <CtrlLib/CtrlLib.h>
#endif
#include <Core/Core.h>
#include <plugin/pcre/Pcre.h>

#include <AI/Engine/CliEngine.h>
#include <AI/Engine/Engines.h>
#include <AI/Engine/PlanSummarizer.h>
#include "WorkSession.h"
#include "Breadcrumb.h"
#include "WorkGraphGenerator.h"
#include "WorkGraphRunner.h"
#include "WorkGraphScorer.h"
#include "StructureTools.h"
#include "VarFileParser.h"

NAMESPACE_UPP

// 1. Data Models
#include "PlanModels.h"
#include "UppParser.h"
#include "RepoScanner.h"
#include "PlanParser.h"

#ifdef flagGUI
// 3. UI
#include "RepoView.h"
#include "PlanView.h"
#include "SessionSelectWindow.h"
#include "AIChatCtrl.h"
#endif

// 4. Global Tool Registration
void RegisterMaestroTools(MaestroToolRegistry& reg);
Value MaestroUpdateTaskStatus(const ValueMap& params);

END_UPP_NAMESPACE

#endif

#endif // MAESTRO_MAESTRO_H
