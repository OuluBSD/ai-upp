#include "dbg.h"

using namespace Upp;

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
	       << "       dbg --backend <name> check\n"
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
	       << "Commands:\n"
	       << "  check  Probe backend toolchain availability\n"
	       << "  run    Launch a program (not implemented yet)\n";
}

static int FindArg(const Vector<String>& args, const char *needle)
{
	for(int i = 0; i < args.GetCount(); i++)
		if(args[i] == needle)
			return i;
	return -1;
}

static bool ParseRunCommand(const Vector<String>& args, int pos, const String& backend_name, DbgLaunchRequest& request, String& error)
{
	bool parsing_run_options = true;

	for(int i = pos; i < args.GetCount(); i++) {
		const String& arg = args[i];

		if(parsing_run_options) {
			if(arg == "--") {
				parsing_run_options = false;
				continue;
			}
			if(arg == "--cwd") {
				if(i + 1 >= args.GetCount()) {
					error = "dbg: missing value for --cwd";
					return false;
				}
				request.working_directory = args[++i];
				continue;
			}
			if(arg == "--env") {
				if(i + 1 >= args.GetCount()) {
					error = "dbg: missing value for --env";
					return false;
				}
				String env = args[++i];
				int eq = env.Find('=');
				if(eq < 1 || eq + 1 >= env.GetCount()) {
					error = "dbg: invalid --env value '" + env + "'";
					return false;
				}
				request.environment.Add(env.Left(eq), env.Mid(eq + 1));
				continue;
			}
			if(arg == "--quiet") {
				request.quiet = true;
				continue;
			}
			if(arg == "--verbose") {
				request.quiet = false;
				continue;
			}
			if(arg.StartsWith("--")) {
				error = "dbg: unknown run option '" + arg + "' for '" + backend_name + "'";
				return false;
			}

			request.executable_path = arg;
			parsing_run_options = false;
			continue;
		}

		if(arg != "--")
			request.arguments.Add(arg);
	}

	if(request.executable_path.IsEmpty()) {
		error = "dbg: missing executable path for '" + backend_name + "'";
		return false;
	}

	return true;
}

static int RunBackendCheck(const DbgBackendInfo& backend)
{
	DbgToolchainStatus status = CheckDbgBackendToolchain(backend.name);
	if(status.available) {
		for(const String& line : status.messages)
			Cout() << line << '\n';
		return 0;
	}

	for(const String& line : status.messages)
		Cerr() << line << '\n';
	return 1;
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
	if(args[pos] == "check") {
		if(pos + 1 != args.GetCount()) {
			Cerr() << "dbg: unexpected arguments after 'check' for '" << backend.name << "'\n";
			return 1;
		}
		return RunBackendCheck(backend);
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
	String error;
	if(!ParseRunCommand(args, pos + 1, backend.name, request, error)) {
		Cerr() << error << "\n";
		return 1;
	}

	One<DbgBackendSession> session = CreateDbgBackendSession(backend.name);
	DbgRunResult result = session ? session->Run(request) : DbgRunResult();
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
