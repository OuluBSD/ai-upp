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
	result.transcript << "executable: " << request.executable_path << '\n';
	if(!request.working_directory.IsEmpty())
		result.transcript << "cwd: " << request.working_directory << '\n';
	for(int i = 0; i < request.arguments.GetCount(); i++)
		result.transcript << "arg[" << i << "]: " << request.arguments[i] << '\n';
	result.transcript << "note: LLDB toolchain/Python setup will be verified in a later task\n";
	return result;
}
