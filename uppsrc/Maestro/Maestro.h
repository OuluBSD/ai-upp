#ifndef _Maestro_Maestro_h_
#define _Maestro_Maestro_h_

#include <CtrlLib/CtrlLib.h>
#include <plugin/pcre/Pcre.h>

NAMESPACE_UPP

// 1. Data Models
#include "PlanModels.h"
#include "UppParser.h"
#include "RepoScanner.h"
#include "PlanParser.h"

// 2. Engine and Tools
#include "Engine.h"
#include "CliEngine.h"
#include "Engines.h"

// 3. UI
#include "RepoView.h"
#include "PlanView.h"
#include "SessionSelectWindow.h"
#include "AIChatCtrl.h"

// 4. Global Tool Registration
void RegisterMaestroTools(MaestroToolRegistry& reg);

END_UPP_NAMESPACE

#endif
