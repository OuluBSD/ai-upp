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

END_UPP_NAMESPACE

#endif