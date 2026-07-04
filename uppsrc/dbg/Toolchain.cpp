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

	if(python_path.IsEmpty()) {
#ifdef PLATFORM_POSIX
		if(!FindExecutableOnPath("python3", python_path))
			FindExecutableOnPath("python", python_path);
#else
		FindExecutableOnPath("python.exe", python_path);
#endif
	}

	if(python_path.IsEmpty())
		return false;

	String out;
	if(Sys("\"" + python_path + "\" --version", out) < 0)
		return false;

	out = TrimBoth(out);
	version = out;
#ifdef PLATFORM_POSIX
	return out.Find("Python 3") >= 0;
#else
	return out.Find("Python 3.11") >= 0;
#endif
}

DbgToolchainStatus CheckDbgBackendToolchain(const String& backend_name)
{
	DbgToolchainStatus status;
	status.backend_name = backend_name;

	if(backend_name == "gdb") {
		String gdb_path;
#ifdef PLATFORM_POSIX
		const char *gdb_exe = "gdb";
#else
		const char *gdb_exe = "gdb.exe";
#endif
		if(FindExecutableOnPath(gdb_exe, gdb_path)) {
			status.available = true;
			status.messages.Add(String("gdb: ") + gdb_exe + " found at " + gdb_path);
			status.messages.Add("gdb: toolchain check passed");
		}
		else {
			status.messages.Add(String("gdb: ") + gdb_exe + " not found on PATH");
		}
		return status;
	}

	if(backend_name == "lldb") {
		String lldb_path;
		String python_path;
		String python_version;

#ifdef PLATFORM_POSIX
		const char *lldb_exe = "lldb";
#else
		const char *lldb_exe = "lldb.exe";
#endif
		bool has_lldb = FindExecutableOnPath(lldb_exe, lldb_path);
		bool has_python = ProbePython311(python_path, python_version);

		if(has_lldb)
			status.messages.Add(String("lldb: ") + lldb_exe + " found at " + lldb_path);
		else {
#ifdef PLATFORM_POSIX
			status.messages.Add("lldb: lldb not found on PATH");
#else
			status.messages.Add("lldb: lldb.exe not found on PATH or in C:\\Program Files\\LLVM\\bin");
#endif
		}

		if(has_python)
			status.messages.Add("lldb: Python found at " + python_path + " (" + python_version + ")");
		else {
#ifdef PLATFORM_POSIX
			status.messages.Add("lldb: Python 3 not found on PATH");
#else
			status.messages.Add("lldb: Python 3.11 not found on PATH and C:\\Python311\\python.exe is missing");
#endif
		}

		status.available = has_lldb && has_python;
		if(status.available)
			status.messages.Add("lldb: toolchain check passed");
		return status;
	}

	if(backend_name == "vs") {
#ifdef PLATFORM_POSIX
		status.messages.Add("vs: Visual Studio backend is not supported on POSIX/Linux");
		status.available = false;
		return status;
#else
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
#endif
	}

	if(backend_name == "java") {
		String jdb_path;
#ifdef PLATFORM_POSIX
		const char *jdb_exe = "jdb";
#else
		const char *jdb_exe = "jdb.exe";
#endif
		if(FindExecutableOnPath(jdb_exe, jdb_path)) {
			status.available = true;
			status.messages.Add(String("java: ") + jdb_exe + " found at " + jdb_path);
			status.messages.Add("java: toolchain check passed");
		}
		else {
			status.messages.Add(String("java: ") + jdb_exe + " not found on PATH");
		}
		return status;
	}

	status.messages.Add(backend_name + ": unknown backend");
	return status;
}

