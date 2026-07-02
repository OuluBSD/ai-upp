#include "dbg.h"

using namespace Upp;

class PlannedDbgBackendSession : public DbgBackendSession {
	String backend_name;

public:
	explicit PlannedDbgBackendSession(const String& name) : backend_name(name) {}

	String GetBackendName() const override { return backend_name; }

	DbgRunResult Run(const DbgLaunchRequest& request) override
	{
		DbgRunResult result;
		result.backend_name = backend_name;
		result.exit_code = 1;
		result.error = backend_name + " backend is not implemented yet";
		result.transcript << "executable: " << request.executable_path << '\n';
		if(!request.working_directory.IsEmpty())
			result.transcript << "cwd: " << request.working_directory << '\n';
		for(int i = 0; i < request.arguments.GetCount(); i++)
			result.transcript << "arg[" << i << "]: " << request.arguments[i] << '\n';
		return result;
	}
};

One<DbgBackendSession> CreateDbgBackendSession(const String& backend_name)
{
	if(backend_name == "gdb")
		return One<DbgBackendSession>(new GdbBackendSession);
	if(backend_name == "lldb")
		return One<DbgBackendSession>(new LldbBackendSession);
	if(backend_name == "vs")
		return One<DbgBackendSession>(new VsBackendSession);
	return One<DbgBackendSession>(new PlannedDbgBackendSession(backend_name));
}
