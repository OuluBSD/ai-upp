#include "aimake.h"

#ifndef bmYEAR
#include <build_info.h>
#endif

bool SilentMode;

String GetUmkFile(const char *fn)
{
	if(FileExists(fn))
		return NormalizePath(fn);
	if(DirectoryExists(fn) || *fn == '.')
		return Null;
	String h = ConfigFile(fn);
	if(FileExists(h))
		return h;
	String cfgdir = GetFileFolder(GetFileFolder(ConfigFile("x")));
	ONCELOCK
		PutVerbose("Config directory: " << cfgdir);
	return GetFileOnPath(fn,
	                     cfgdir + "/aimk" + ';' +
	                     cfgdir + "/umk" + ';' +
	                     cfgdir + "/theide" + ';' +
	                     cfgdir + "/ide" + ';' +
	                     GetHomeDirectory() + ';' +
	                     GetFileFolder(GetExeFilePath()));
}

String GetBuildMethodPath(String method)
{
	if(GetFileExt(method) != ".bm")
		method << ".bm";
	return GetUmkFile(method);
}

String Ide::GetDefaultMethod()
{
	return "GCC";
}

String ReplaceMethodDir(String paths, const String& method_dir)
{
	constexpr const char* METHOD_DIR = "${METHOD_DIR}";
	
	if (paths.Find(METHOD_DIR) == -1) {
		return paths;
	}
	paths.Replace(METHOD_DIR, method_dir);
	return paths;
}

VectorMap<String, String> Ide::GetMethodVars(const String& method)
{
	VectorMap<String, String> map;
	LoadVarFile(GetMethodName(method), map);
	
	const String method_dir = GetFileFolder(method);
	const Vector<String> categories_with_method_dir = {"PATH", "INCLUDE", "LIB" };
	for (const auto& category : categories_with_method_dir) {
		map.GetAdd(category) = ReplaceMethodDir(map.Get(category), method_dir);
	}

	return map;
}

String Ide::GetMethodName(const String& method)
{
	return GetBuildMethodPath(method);
}

void Puts(const char *s)
{
	if(!SilentMode)
		Cout() << s;
}

String GetAndroidSDKPath()
{
	return String();
}

#ifdef flagMAIN

String GenerateVersionNumber()
{
#ifdef bmGIT_REVCOUNT
	return AsString(atoi(bmGIT_REVCOUNT) + 2270);
#endif
	return "";
}

void SetupUmkUppHub()
{
	String cfgdir = GetFileFolder(GetFileFolder(ConfigFile("x")));
	for(const char *q : { "aimk", "umk", "theide", "ide" }) {
		String dir = cfgdir + "/" + q + "/UppHub";
		if(DirectoryExists(dir)) {
			for(FindFile ff(dir + "/*"); ff; ff.Next())
				if(ff.IsFolder() && *ff.GetName() != '.') {
					OverrideHubDir(dir);
					return;
				}
		}
	}
}

void ShowHelp(const String& version)
{
	Puts(
		"aimk (AI Make) " + version + " - AI-friendly U++ build tool\n\n"
		"USAGE:\n"
		"  aimk --package=<name> [OPTIONS]\n"
		"  aimk -p <name> [OPTIONS]\n\n"
		"REQUIRED:\n"
		"  -p, --package=NAME         Main package to build (e.g., MyApp)\n\n"
		"COMMON OPTIONS:\n"
		"  -a, --assembly=PATHS       Assembly directories (comma or colon separated)\n"
		"                             Default: upptst,examples,tutorial,reference,uppsrc\n"
		"  -m, --method=NAME          Build method (GCC, CLANG, MSC, etc.)\n"
		"                             Default: CLANG\n"
		"  -o, --output=PATH          Output executable path\n"
		"  -f, --flags=FLAGS          Build flags (comma separated: GUI,SHARED,etc.)\n"
		"  -c, --config=NAME          Build configuration (debug, release)\n"
		"                             Default: debug\n\n"
		"BUILD OPTIONS:\n"
		"  --clean                    Clean before building\n"
		"  --blitz                    Enable blitz build\n"
		"  --no-blitz                 Disable blitz build (default)\n"
		"  --static                   Static linking\n"
		"  --shared                   Shared linking (default)\n"
		"  --threads=N                Number of parallel build threads\n"
		"                             Default: CPU core count\n"
		"  --verbose                  Verbose build output\n"
		"  --silent                   Minimal output\n\n"
		"EXPORT OPTIONS:\n"
		"  --makefile[=PATH]          Generate Makefile\n"
		"  --export-project=PATH      Export project files\n"
		"  --export-all=PATH          Export all project files\n"
		"  --compile-commands         Generate compile_commands.json\n\n"
		"RUN OPTIONS:\n"
		"  --run [ARGS...]            Run executable after successful build\n\n"
		"OTHER OPTIONS:\n"
		"  -h, --help                 Show this help message\n"
		"  -v, --version              Show version information\n\n"
		"EXAMPLES:\n"
		"  # Simple build\n"
		"  aimk --package=Bombs\n\n"
		"  # Build with custom assembly and flags\n"
		"  aimk -p MyApp -a ~/myapp/src,uppsrc -f GUI,SHARED -o ~/bin/myapp\n\n"
		"  # Clean build with CLANG, 8 threads\n"
		"  aimk -p MyApp --clean --method=CLANG --threads=8\n\n"
		"  # Build and run\n"
		"  aimk -p MyApp --run arg1 arg2\n\n"
		"  # Generate compile_commands.json for IDE\n"
		"  aimk -p MyApp --compile-commands\n\n"
		"For more information: https://www.ultimatepp.org/\n"
	);
}

CONSOLE_APP_MAIN
{
	SetConfigName("theide");

#ifdef PLATFORM_POSIX
	setlinebuf(stdout);
	CreateBuildMethods();
#endif

	Ide ide;
	SetTheIde(&ide);
	ide.console.SetSlots(CPU_Cores());
	ide.console.console = true;

	ide.debug.def.blitz = ide.release.def.blitz = 0;
	ide.debug.def.debug = 2;
	ide.release.def.debug = 0;
	ide.debug.package.Clear();
	ide.release.package.Clear();
	ide.debug.linkmode = ide.release.linkmode = 0;
	ide.release.createmap = ide.debug.createmap = false;
	ide.targetmode = 0;
	ide.use_target = false;
	ide.makefile_svn_revision = false;
	bool clean = false;
	bool makefile = false;
	bool ccfile = false;
	bool deletedir = true;
	int  exporting = 0;
	bool run = false;
	bool auto_hub = false;
	bool update_hub = false;
	bool flatpak_build = !GetEnv("FLATPAK_ID").IsEmpty();
	String mkf;

	// New named parameters
	String package_name;
	String assembly_paths;
	String build_method;
	String output_path;
	String build_flags;
	bool show_help = false;
	bool show_version = false;

	Vector<String> runargs;

	const Vector<String>& args = CommandLine();

	// Parse command line arguments
	for(int i = 0; i < args.GetCount(); i++) {
		String a = args[i];

		// Handle --long-option or --long-option=value
		if(a.StartsWith("--")) {
			String opt = a.Mid(2);
			String value;
			int eq = opt.Find('=');
			if(eq >= 0) {
				value = opt.Mid(eq + 1);
				opt = opt.Left(eq);
			}

			if(opt == "help") {
				show_help = true;
			}
			else if(opt == "version") {
				show_version = true;
			}
			else if(opt == "package") {
				if(eq >= 0)
					package_name = value;
				else if(i + 1 < args.GetCount())
					package_name = args[++i];
			}
			else if(opt == "assembly") {
				if(eq >= 0)
					assembly_paths = value;
				else if(i + 1 < args.GetCount())
					assembly_paths = args[++i];
			}
			else if(opt == "method") {
				if(eq >= 0)
					build_method = value;
				else if(i + 1 < args.GetCount())
					build_method = args[++i];
			}
			else if(opt == "output") {
				if(eq >= 0)
					output_path = value;
				else if(i + 1 < args.GetCount())
					output_path = args[++i];
			}
			else if(opt == "flags") {
				if(eq >= 0)
					build_flags = value;
				else if(i + 1 < args.GetCount())
					build_flags = args[++i];
			}
			else if(opt == "config") {
				if(eq >= 0) {
					if(value == "release")
						ide.targetmode = 1;
				}
				else if(i + 1 < args.GetCount()) {
					if(args[i + 1] == "release")
						ide.targetmode = 1;
					i++;
				}
			}
			else if(opt == "clean") {
				clean = true;
			}
			else if(opt == "blitz") {
				ide.release.def.blitz = ide.debug.def.blitz = 1;
			}
			else if(opt == "no-blitz") {
				ide.release.def.blitz = ide.debug.def.blitz = 0;
			}
			else if(opt == "static") {
				ide.debug.linkmode = ide.release.linkmode = 1;
			}
			else if(opt == "shared") {
				ide.debug.linkmode = ide.release.linkmode = 2;
			}
			else if(opt == "threads") {
				int n = 0;
				if(eq >= 0)
					n = atoi(value);
				else if(i + 1 < args.GetCount())
					n = atoi(args[++i]);
				if(n > 0) {
					n = minmax(n, 1, 256);
					PutVerbose("Build threads: " + AsString(n));
					ide.console.SetSlots(n);
				}
			}
			else if(opt == "verbose") {
				ide.console.verbosebuild = true;
			}
			else if(opt == "silent") {
				SilentMode = true;
			}
			else if(opt == "makefile") {
				makefile = true;
				if(eq >= 0)
					mkf = NormalizePath(value);
			}
			else if(opt == "export-project") {
				exporting = 1;
				if(eq >= 0)
					mkf = value;
				else if(i + 1 < args.GetCount())
					mkf = args[++i];
			}
			else if(opt == "export-all") {
				exporting = 2;
				if(eq >= 0)
					mkf = value;
				else if(i + 1 < args.GetCount())
					mkf = args[++i];
			}
			else if(opt == "compile-commands") {
				ccfile = true;
			}
			else if(opt == "run") {
				run = true;
				// Collect remaining args for the program
				for(int j = i + 1; j < args.GetCount(); j++)
					runargs.Add(args[j]);
				break;
			}
			else {
				Puts("Unknown option: --" + opt + "\n");
				Puts("Use --help for usage information\n");
				SetExitCode(3);
				return;
			}
		}
		// Handle -short options
		else if(a.StartsWith("-") && a.GetLength() > 1) {
			char opt = a[1];
			String value = a.GetLength() > 2 ? a.Mid(2) : String();

			if(opt == 'h') {
				show_help = true;
			}
			else if(opt == 'v') {
				show_version = true;
			}
			else if(opt == 'p') {
				if(!value.IsEmpty())
					package_name = value;
				else if(i + 1 < args.GetCount())
					package_name = args[++i];
			}
			else if(opt == 'a') {
				if(!value.IsEmpty())
					assembly_paths = value;
				else if(i + 1 < args.GetCount())
					assembly_paths = args[++i];
			}
			else if(opt == 'm') {
				if(!value.IsEmpty())
					build_method = value;
				else if(i + 1 < args.GetCount())
					build_method = args[++i];
			}
			else if(opt == 'o') {
				if(!value.IsEmpty())
					output_path = value;
				else if(i + 1 < args.GetCount())
					output_path = args[++i];
			}
			else if(opt == 'f') {
				if(!value.IsEmpty())
					build_flags = value;
				else if(i + 1 < args.GetCount())
					build_flags = args[++i];
			}
			else if(opt == 'c') {
				if(!value.IsEmpty()) {
					if(value == "release")
						ide.targetmode = 1;
				}
				else if(i + 1 < args.GetCount()) {
					if(args[i + 1] == "release")
						ide.targetmode = 1;
					i++;
				}
			}
			else {
				Puts("Unknown option: -" + String(opt, 1) + "\n");
				Puts("Use --help for usage information\n");
				SetExitCode(3);
				return;
			}
		}
	}

	String version = GenerateVersionNumber();

	// Handle --help and --version
	if(show_help) {
		ShowHelp(version);
		SetExitCode(0);
		return;
	}

	if(show_version) {
		Puts("aimk (AI Make) " + version + "\n");
		SetExitCode(0);
		return;
	}

	// Check if package is specified
	if(package_name.IsEmpty()) {
		Puts("Error: Package name is required\n\n");
		ShowHelp(version);
		SetExitCode(1);
		return;
	}

	// Set defaults
	if(assembly_paths.IsEmpty())
		assembly_paths = "upptst,examples,tutorial,reference,uppsrc";
	if(build_method.IsEmpty())
		build_method = "CLANG";

	// Apply build flags
	if(!build_flags.IsEmpty())
		ide.mainconfigparam = Filter(build_flags, [](int c) { return c == ',' ? ' ' : c; });

	PutVerbose("Package: " + package_name);
	PutVerbose("Assembly: " + assembly_paths);
	PutVerbose("Method: " + build_method);
	if(!build_flags.IsEmpty())
		PutVerbose("Flags: " + build_flags);

	if(auto_hub)
		DeleteFolderDeep(GetHubDir());
	else
		SetupUmkUppHub();

	// Process assembly paths
	String v = GetUmkFile(assembly_paths + ".var");
	if(IsNull(v)) {
	#ifdef PLATFORM_POSIX
		Vector<String> h = Split(assembly_paths, [](int c) { return c == ':' || c == ',' ? c : 0; });
	#else
		Vector<String> h = Split(assembly_paths, ',');
	#endif
		for(int i = 0; i < h.GetCount(); i++)
			h[i] = GetFullPath(TrimBoth(h[i]));
		String x = Join(h, ";");
		SetVar("UPP", x, false);
		PutVerbose("Inline assembly: " + x);
		String outdir = GetDefaultUppOut();
		if (flatpak_build) {
			outdir = GetExeFolder() + DIR_SEPS + ".cache" + DIR_SEPS + "upp.out";
		}
		RealizeDirectory(outdir);
		SetVar("OUTPUT", outdir, false);
	}
	else {
		if(!LoadVars(v)) {
			Puts("Invalid assembly\n");
			SetExitCode(2);
			return;
		}
		PutVerbose("Assembly file: " + v);
		PutVerbose("Assembly: " + GetVar("UPP"));
		PutVerbose("AI overlay: " + GetVar("AI"));
	}
	PutVerbose("Output directory: " + GetUppOut());
	ide.main = package_name;
	v = SourcePath(ide.main, GetFileTitle(ide.main) + ".upp");
	PutVerbose("Main package: " + v);
		if(!FileExists(v)) {
			Puts("Package " + ide.main + " does not exist\n");
			SetExitCode(2);
			return;
		}
		if(auto_hub || update_hub) {
			if(!UppHubAuto(ide.main)) {
				SetExitCode(6);
				return;
			}
			if (update_hub)
				UppHubUpdate(ide.main);
		}
		ide.wspc.Scan(ide.main);
		const Workspace& wspc = ide.IdeWorkspace();
		if(!wspc.GetCount()) {
			Puts("Empty assembly\n");
			SetExitCode(4);
			return;
		}
		Index<String> missing;
		for(int i = 0; i < wspc.GetCount(); i++) {
			String p = wspc[i];
			if(!FileExists(PackageFile(p)))
				missing.FindAdd(p);
		}
		if(missing.GetCount()) {
			Puts("Missing package(s): " << Join(missing.GetKeys(), " ") << "\n");
			SetExitCode(5);
			return;
		}
		if(IsNull(ide.mainconfigparam)) {
			const Array<Package::Config>& f = wspc.GetPackage(0).config;
			if(f.GetCount())
				ide.mainconfigparam = f[0].param;
		}
	PutVerbose("Build flags: " << ide.mainconfigparam);
	String bp = GetBuildMethodPath(build_method);
	PutVerbose("Build method: " + bp);
	if(bp.GetCount() == 0) {
		SilentMode = false;
		Puts("Invalid build method: " + build_method + "\n");
		SetExitCode(3);
		return;
	}

	if(!output_path.IsEmpty()) {
		ide.debug.target_override = ide.release.target_override = true;
		ide.debug.target = ide.release.target = NormalizePath(output_path);
		PutVerbose("Output path: " << ide.debug.target);
	}

	ide.method = bp;

	if(ccfile) {
		ide.SaveCCJ(GetFileDirectory(PackageFile(ide.main)) + "compile_commands.json", false);
		SetExitCode(0);
		return;
	}

	if(clean)
		ide.Clean();
	if(exporting) {
		mkf = GetFullPath(mkf);
		Cout() << mkf << '\n';
		RealizeDirectory(mkf);
		if(makefile)
			ide.ExportMakefile(mkf);
		else
			ide.ExportProject(mkf, exporting == 2, deletedir);
	}
	else
	if(makefile) {
		ide.SaveMakeFile(IsNull(mkf) ? "Makefile" : mkf, false);
		SetExitCode(0);
	}
	else
	if(ide.Build()) {
		SetExitCode(0);
		if(run) {
			Vector<char *> args;
			Vector<Buffer<char>> buffer;
			auto Add = [&](const String& s) {
				auto& b = buffer.Add();
				b.Alloc(s.GetCount() + 1);
				memcpy(b, s, s.GetCount() + 1);
				args.Add(b);
			};
			Add(ide.target);
			for(const String& s : runargs)
				Add(s);
			args.Add(NULL);
			SetExitCode((int)execv(ide.target, args.begin()));
		}
	}
	else
		SetExitCode(1);
}

#endif
