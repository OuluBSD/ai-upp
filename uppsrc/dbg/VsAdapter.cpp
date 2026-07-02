#include "dbg.h"

using namespace Upp;

String VsBackendSession::GetBackendName() const
{
	return "vs";
}

DbgRunResult VsBackendSession::Run(const DbgLaunchRequest& request)
{
	DbgRunResult result;
	result.backend_name = "vs";
	result.exit_code = 1;
	result.error = "vs backend is not implemented yet";
	result.transcript << "executable: " << request.executable_path << '\n';
	if(!request.working_directory.IsEmpty())
		result.transcript << "cwd: " << request.working_directory << '\n';
	for(int i = 0; i < request.arguments.GetCount(); i++)
		result.transcript << "arg[" << i << "]: " << request.arguments[i] << '\n';
	result.transcript << "note: Visual Studio / PDB backend integration will be verified later\n";
	return result;
}
