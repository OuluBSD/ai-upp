#include "dbg.h"

using namespace Upp;

void AppendDbgLaunchRequestTranscript(String& transcript, const DbgLaunchRequest& request)
{
	transcript << "executable: " << request.executable_path << '\n';
	if(!request.working_directory.IsEmpty())
		transcript << "cwd: " << request.working_directory << '\n';
	for(int i = 0; i < request.environment.GetCount(); i++)
		transcript << "env: " << request.environment.GetKey(i) << '=' << request.environment[i] << '\n';
	for(int i = 0; i < request.arguments.GetCount(); i++)
		transcript << "arg[" << i << "]: " << request.arguments[i] << '\n';
	transcript << "mode: " << (request.quiet ? "quiet" : "verbose") << '\n';
}
