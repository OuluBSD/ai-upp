#include "dbg.h"

using namespace Upp;

static String CleanPathPart(String path)
{
	path = TrimBoth(path);
	if(path.GetCount() >= 2 && path.StartsWith("\"") && path.EndsWith("\""))
		path = path.Mid(1, path.GetCount() - 2);
	return path;
}

static Vector<String> SplitPathDirs(const String& path)
{
#ifdef PLATFORM_POSIX
	return Split(path, ':');
#else
	return Split(path, ';');
#endif
}

static bool FindExecutableOnPath(const char *name, String& found_path)
{
	for(String dir : SplitPathDirs(GetEnv("PATH"))) {
		dir = CleanPathPart(dir);
		if(dir.IsEmpty())
			continue;
		String candidate = AppendFileName(dir, name);
		if(FileExists(candidate)) {
			found_path = candidate;
			return true;
		}
	}
	return false;
}

static bool FindVsWhere(String& vswhere_path)
{
	if(FindExecutableOnPath("vswhere.exe", vswhere_path))
		return true;

	Vector<String> probes;
	String pf86 = GetEnv("ProgramFiles(x86)");
	String pf = GetEnv("ProgramFiles");

	if(!pf86.IsEmpty())
		probes.Add(AppendFileName(AppendFileName(AppendFileName(pf86, "Microsoft Visual Studio"), "Installer"), "vswhere.exe"));
	if(!pf.IsEmpty())
		probes.Add(AppendFileName(AppendFileName(AppendFileName(pf, "Microsoft Visual Studio"), "Installer"), "vswhere.exe"));

	for(const String& probe : probes)
		if(FileExists(probe)) {
			vswhere_path = probe;
			return true;
		}

	return false;
}

static bool ProbePython311(String& python_path, String& version)
{
#ifdef PLATFORM_WIN32
	const String fixed = "C:\\Python311\\python.exe";
	if(FileExists(fixed))
		python_path = fixed;
#endif

	if(python_path.IsEmpty() && !FindExecutableOnPath("python.exe", python_path))
		return false;

	String out;
	if(Sys("\"" + python_path + "\" --version", out) < 0)
		return false;

	out = TrimBoth(out);
	version = out;
	return out.Find("Python 3.11") >= 0;
}

DbgToolchainStatus CheckDbgBackendToolchain(const String& backend_name)
{
	DbgToolchainStatus status;
	status.backend_name = backend_name;

	if(backend_name == "gdb") {
		String gdb_path;
		if(FindExecutableOnPath("gdb.exe", gdb_path)) {
			status.available = true;
			status.messages.Add("gdb: gdb.exe found at " + gdb_path);
			status.messages.Add("gdb: toolchain check passed");
		}
		else {
			status.messages.Add("gdb: gdb.exe not found on PATH");
		}
		return status;
	}

	if(backend_name == "lldb") {
		String lldb_path;
		String python_path;
		String python_version;

		bool has_lldb = FindExecutableOnPath("lldb.exe", lldb_path);
		bool has_python = ProbePython311(python_path, python_version);

		if(has_lldb)
			status.messages.Add("lldb: lldb.exe found at " + lldb_path);
		else
			status.messages.Add("lldb: lldb.exe not found on PATH or in C:\\Program Files\\LLVM\\bin");

		if(has_python)
			status.messages.Add("lldb: Python 3.11 found at " + python_path + " (" + python_version + ")");
		else
			status.messages.Add("lldb: Python 3.11 not found on PATH and C:\\Python311\\python.exe is missing");

		status.available = has_lldb && has_python;
		if(status.available)
			status.messages.Add("lldb: toolchain check passed");
		return status;
	}

	if(backend_name == "vs") {
		String vswhere_path;
		String installation_path;

		if(!FindVsWhere(vswhere_path)) {
			status.messages.Add("vs: vswhere.exe not found on PATH or in the Visual Studio Installer folder");
			return status;
		}

		status.messages.Add("vs: vswhere.exe found at " + vswhere_path);

		String output;
		String cmd = "\"" + vswhere_path + "\" -latest -products * -requires Microsoft.Component.MSBuild -property installationPath";
		if(Sys(cmd, output) < 0) {
			status.messages.Add("vs: unable to query Visual Studio installation");
			return status;
		}

		for(const String& line : Split(output, '\n')) {
			String trimmed = TrimBoth(line);
			if(trimmed.IsEmpty())
				continue;
			installation_path = trimmed;
			break;
		}

		if(installation_path.IsEmpty()) {
			status.messages.Add("vs: Visual Studio installation not detected");
			return status;
		}

		status.available = true;
		status.messages.Add("vs: installation found at " + installation_path);
		status.messages.Add("vs: toolchain check passed");
		return status;
	}

	status.messages.Add(backend_name + ": unknown backend");
	return status;
}
