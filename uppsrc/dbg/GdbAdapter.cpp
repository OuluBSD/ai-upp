#include "dbg.h"

using namespace Upp;

String GdbBackendSession::GetBackendName() const
{
	return "gdb";
}

DbgRunResult GdbBackendSession::Run(const DbgLaunchRequest& request)
{
	DbgRunResult result;
	result.backend_name = "gdb";
	result.exit_code = 1;
	result.error = "gdb backend is not implemented yet";
	AppendDbgLaunchRequestTranscript(result.transcript, request);
	return result;
}
