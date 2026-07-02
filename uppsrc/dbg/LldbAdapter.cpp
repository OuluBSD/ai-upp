#include "dbg.h"

using namespace Upp;

String LldbBackendSession::GetBackendName() const
{
	return "lldb";
}

DbgRunResult LldbBackendSession::Run(const DbgLaunchRequest& request)
{
	DbgRunResult result;
	result.backend_name = "lldb";
	result.exit_code = 1;
	result.error = "lldb backend is not implemented yet";
	AppendDbgLaunchRequestTranscript(result.transcript, request);
	result.transcript << "note: LLDB toolchain/Python setup will be verified in a later task\n";
	return result;
}
