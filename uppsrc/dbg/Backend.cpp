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

Vector<DbgBackendInfo> GetPlannedDbgBackends()
{
	Vector<DbgBackendInfo> backends;
	DbgBackendInfo& vs = backends.Add();
	vs.name = "vs";
	vs.description = "Visual Studio backend (planned)";
	DbgBackendInfo& gdb = backends.Add();
	gdb.name = "gdb";
	gdb.description = "GDB backend (planned)";
	DbgBackendInfo& lldb = backends.Add();
	lldb.name = "lldb";
	lldb.description = "LLDB backend (planned)";
	return backends;
}

const DbgBackendInfo *FindPlannedDbgBackend(const String& name)
{
	static Vector<DbgBackendInfo> backends = GetPlannedDbgBackends();
	for(DbgBackendInfo& backend : backends)
		if(backend.name == name)
			return &backend;
	return NULL;
}

String GetPlannedDbgBackendList()
{
	const Vector<DbgBackendInfo> backends = GetPlannedDbgBackends();
	String out;
	for(int i = 0; i < backends.GetCount(); i++) {
		if(i)
			out << ", ";
		out << backends[i].name;
	}
	return out;
}

static void PrintHelp()
{
	Cout() << "Usage: dbg [--help] [--backends]\n"
	       << "       dbg --backend <name> --help\n"
	       << "       dbg --backend <name> run <program> [args...]\n"
	       << "\n"
	       << "  --backends  List planned debugger backends.\n"
	       << "  --help      Show this help text.\n"
	       << "\n"
	       << "Planned backends: " << GetPlannedDbgBackendList() << "\n";
}

static void PrintBackends()
{
	Cout() << "Planned backends: " << GetPlannedDbgBackendList() << "\n";
}

static void PrintBackendHelp(const DbgBackendInfo& backend)
{
	Cout() << "Backend: " << backend.name << "\n"
	       << "Description: " << backend.description << "\n"
	       << "Status: not implemented yet\n";
}

static int FindArg(const Vector<String>& args, const char *needle)
{
	for(int i = 0; i < args.GetCount(); i++)
		if(args[i] == needle)
			return i;
	return -1;
}

static int RunBackendCommand(const DbgBackendInfo& backend, const Vector<String>& args, int pos)
{
	if(pos >= args.GetCount()) {
		Cerr() << "dbg: missing backend command for '" << backend.name << "'\n";
		return 1;
	}
	if(args[pos] == "--help") {
		PrintBackendHelp(backend);
		return 0;
	}
	if(args[pos] != "run") {
		Cerr() << "dbg: unknown backend command '" << args[pos] << "' for '" << backend.name << "'\n";
		PrintBackendHelp(backend);
		return 1;
	}
	if(pos + 1 >= args.GetCount()) {
		Cerr() << "dbg: missing executable path for '" << backend.name << "'\n";
		return 1;
	}

	DbgLaunchRequest request;
	request.executable_path = args[pos + 1];
	for(int i = pos + 2; i < args.GetCount(); i++)
		request.arguments.Add(args[i]);

	DbgRunResult result;
	if(backend.name == "gdb") {
		GdbBackendSession session;
		result = session.Run(request);
	}
	else {
		PlannedDbgBackendSession session(backend.name);
		result = session.Run(request);
	}
	if(!result.error.IsEmpty())
		Cerr() << "dbg: " << result.error << "\n";
	return IsNull(result.exit_code) ? 1 : result.exit_code;
}

int RunDbgCli(const Vector<String>& args)
{
	int backend_pos = FindArg(args, "--backend");
	if(backend_pos >= 0) {
		if(backend_pos + 1 >= args.GetCount()) {
			Cerr() << "dbg: missing backend name\n";
			PrintHelp();
			return 1;
		}

		String backend_name = args[backend_pos + 1];
		const DbgBackendInfo *backend = FindPlannedDbgBackend(backend_name);
		if(!backend) {
			Cerr() << "dbg: unknown backend '" << backend_name << "'\n";
			Cerr() << "dbg: available backends: " << GetPlannedDbgBackendList() << "\n";
			return 1;
		}

		return RunBackendCommand(*backend, args, backend_pos + 2);
	}

	if(args.IsEmpty() || FindArg(args, "--help") >= 0 || FindArg(args, "-h") >= 0) {
		PrintHelp();
		return 0;
	}
	if(FindArg(args, "--backends") >= 0) {
		PrintBackends();
		return 0;
	}

	PrintHelp();
	return 0;
}
