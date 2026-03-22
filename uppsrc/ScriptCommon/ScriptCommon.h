#ifndef _ScriptCommon_ScriptCommon_h_
#define _ScriptCommon_ScriptCommon_h_

#include <Core/Core.h>
#include <ByteVM/ByteVM.h>
#include <ByteVM/PyBindings.h>
#include <Form/Common/FormLayout.hpp>
	
NAMESPACE_UPP

#include "IDESettings.h"
#include "PathManager.h"
#include "Linter.h"
#include "RunManager.h"
#include "ScriptServices.h"
#include "ScriptMcpProtocol.h"
#include "ScriptMcpCapabilities.h"
#include "ScriptMcpHandlers.h"
#include "PluginInterfaces.h"
#include "GameStatePlugin.h"
#include "CardGamePlugin.h"

END_UPP_NAMESPACE

#endif
