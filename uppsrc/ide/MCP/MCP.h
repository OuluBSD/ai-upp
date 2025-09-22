#ifndef _ide_MCP_MCP_h_
#define _ide_MCP_MCP_h_

#include <Core/Core.h>
#include <CtrlLib/CtrlLib.h>

#define NAMESPACE_UPP namespace Upp {
#define END_UPP_NAMESPACE }


NAMESPACE_UPP

#include "Protocol.h"

bool StartMcpServer(const McpConfig& cfg);
void StopMcpServer();
bool McpIsRunning();

// Aggregated headers
#include "Server.h"
#include "WorkspaceBridge.h"

END_UPP_NAMESPACE

#endif
