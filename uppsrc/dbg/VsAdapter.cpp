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
	AppendDbgLaunchRequestTranscript(result.transcript, request);
	result.transcript << "note: Visual Studio / PDB backend integration will be verified later\n";
	return result;
}
