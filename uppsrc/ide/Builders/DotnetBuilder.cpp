#include "Builders.h"
#include "UwpTemplates.brc"

namespace {

struct DotnetRuntimeVersion {
	int    major = 0;
	int    minor = 0;
	int    patch = 0;
	String version;
	String root;
};

static bool ParseDotnetVersion(const String& name, int& major, int& minor, int& patch)
{
	String clean;
	for(int i = 0; i < name.GetCount(); i++) {
		int c = name[i];
		if(IsDigit(c) || c == '.')
			clean.Cat(c);
		else
			break;
	}
	if(clean.IsEmpty())
		return false;

	Vector<String> parts = Split(clean, '.');
	if(parts.GetCount() < 2)
		return false;

	major = ScanInt(parts[0]);
	minor = ScanInt(parts[1]);
	patch = parts.GetCount() >= 3 ? ScanInt(parts[2]) : 0;
	return major > 0;
}

static bool IsBetterVersion(int major, int minor, int patch, const DotnetRuntimeVersion& best)
{
	if(major != best.major)
		return major > best.major;
	if(minor != best.minor)
		return minor > best.minor;
	return patch > best.patch;
}

static void AddDotnetRoot(Index<String>& roots, const String& root)
{
	if(!IsNull(root) && DirectoryExists(root))
		roots.FindAdd(NativePath(root));
}

static void AddDotnetRoots(Index<String>& roots)
{
	AddDotnetRoot(roots, GetEnv("DOTNET_ROOT"));
	AddDotnetRoot(roots, GetEnv("DOTNET_ROOT(x64)"));
	AddDotnetRoot(roots, "/usr/share/dotnet");
	AddDotnetRoot(roots, "/usr/lib/dotnet");
	if(DirectoryExists("/opt")) {
		FindFile ff(AppendFileName("/opt", "dotnet-sdk-bin-*"));
		while(ff) {
			if(ff.IsFolder())
				AddDotnetRoot(roots, ff.GetPath());
			ff.Next();
		}
	}
}

static String DetectDotnetTargetFramework(String& runtime_root, String& runtime_version)
{
	Index<String> roots;
	AddDotnetRoots(roots);

	DotnetRuntimeVersion best;
	for(int i = 0; i < roots.GetCount(); i++) {
		String shared_dir = AppendFileName(roots[i], AppendFileName("shared", "Microsoft.NETCore.App"));
		if(!DirectoryExists(shared_dir))
			continue;
		FindFile ff(AppendFileName(shared_dir, "*"));
		while(ff) {
			if(ff.IsFolder()) {
				int major = 0;
				int minor = 0;
				int patch = 0;
				if(ParseDotnetVersion(ff.GetName(), major, minor, patch)) {
					if(IsBetterVersion(major, minor, patch, best)) {
						best.major = major;
						best.minor = minor;
						best.patch = patch;
						best.version = ff.GetName();
						best.root = roots[i];
					}
				}
			}
			ff.Next();
		}
	}

	if(best.major == 0)
		return String();
	runtime_root = best.root;
	runtime_version = best.version;
	return Format("net%d.%d", best.major, best.minor);
}

}

String DotnetBuilder::GetTargetExt() const
{
	return ".dll";
}

void DotnetBuilder::AddFlags(Index<String>& cfg)
{
	cfg.FindAdd("DOTNET");
}

bool DotnetBuilder::BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>&,
	String& linkoptions, const Vector<String>&, const Vector<String>&, int)
{
	if(!dotnet_started) {
		project_files.Clear();
		dotnet_started = true;
	}

	int time = msecs();
	Package pkg;
	pkg.Load(PackageFile(package));
	String packagedir = PackageDirectory(package);

	ChDir(packagedir);
	PutVerbose("cd " + packagedir);

	bool error = false;
	int staged = 0;

	for(int i = 0; i < pkg.GetCount(); i++) {
		if(!IdeIsBuilding())
			return false;
		if(pkg[i].separator)
			continue;

		Vector<String> srcfile = CustomStep(pkg[i], package, error);
		if(srcfile.IsEmpty())
			error = true;

		for(int j = 0; j < srcfile.GetCount(); j++) {
			if(!IdeIsBuilding())
				return false;

			bool updated = false;
			String rel = NativePath(MakeUwpRelativePath(PackageDirectory(package), srcfile[j]));
			String target = AppendFileName(outdir, AppendFileName(package, rel));
			CollectUwpProjectFiles(project_files, srcfile[j], target);
			if(!StageUwpFile(srcfile[j], target, updated))
				error = true;
			else if(updated)
				staged++;
		}
	}

	if(staged > 0)
		PutConsole(String().Cat() << staged << " file(s) staged in " << GetPrintTime(time));

	linkfile.Add(outdir);
	linkoptions << ' ' << Gather(pkg.link, config.GetKeys());
	return !error;
}

bool DotnetBuilder::Link(const Vector<String>&, const String&, bool)
{
	if(project_files.IsEmpty()) {
		PutConsole("DOTNET: no sources staged.");
		dotnet_started = false;
		return true;
	}

	String solution_dir = outdir;
	String project_name = GetFileTitle(mainpackage);
	if(IsNull(project_name))
		project_name = "DotnetApp";

	String project_guid = UwpGuidString("DOTNET:" + project_name);
	String project_type_guid = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC";
	String runtime_root;
	String runtime_version;
	String target_framework = DetectDotnetTargetFramework(runtime_root, runtime_version);
	if(IsNull(target_framework)) {
		target_framework = "net8.0";
	}
	else {
		PutConsole(Format("DOTNET: detected runtime %s at %s", runtime_version, runtime_root));
	}

	Index<String> cscompile;
	Index<String> none;
	for(int i = 0; i < project_files.GetCount(); i++) {
		const String& path = project_files[i];
		if(IsCSharpFile(path))
			cscompile.FindAdd(path);
		else
			none.FindAdd(path);
	}

	VectorMap<String, String> sln_tokens;
	sln_tokens.Add("PROJECT_NAME", project_name);
	sln_tokens.Add("PROJECT_GUID", project_guid);
	sln_tokens.Add("PROJECT_TYPE_GUID", project_type_guid);
	sln_tokens.Add("PROJECT_FILE", project_name + ".csproj");

	String sln = ReplaceUwpTokens(UwpTemplate(uwp_sln_tpl, uwp_sln_tpl_length), sln_tokens);
	SaveFile(AppendFileName(solution_dir, project_name + ".sln"), sln);

	VectorMap<String, String> proj_tokens;
	proj_tokens.Add("ROOT_NAMESPACE", XmlEscape(project_name));
	proj_tokens.Add("TARGET_FRAMEWORK", XmlEscape(target_framework));
	proj_tokens.Add("CSCOMPILE", String());
	proj_tokens.Add("NONEITEMS", MakeItemList(none, solution_dir, "None"));

	String csproj = ReplaceUwpTokens(UwpTemplate(dotnet_csproj_tpl, dotnet_csproj_tpl_length), proj_tokens);
	SaveFile(AppendFileName(solution_dir, project_name + ".csproj"), csproj);

	String conf = HasFlag("DEBUG") ? "Debug" : "Release";
	String sln_path = AppendFileName(solution_dir, project_name + ".sln");
	String cmd;
	cmd << "dotnet build " << GetPathQ(sln_path) << " -c " << conf;
	PutConsole("DOTNET: building solution with dotnet...");
	if(Execute(cmd) != 0) {
		PutConsole("DOTNET: dotnet build failed.");
		dotnet_started = false;
		return false;
	}

	String dll_path = AppendFileName(
		solution_dir,
		AppendFileName(
			"bin",
			AppendFileName(
				"x64",
				AppendFileName(
					conf,
					AppendFileName(
						target_framework,
						project_name + ".dll")))));
	PutConsole(Format("DOTNET: run with: dotnet %s", dll_path));
	PutConsole(Format("DOTNET: wrote Visual Studio project files to %s", solution_dir));
	dotnet_started = false;
	return true;
}

static Builder *CreateDotnetBuilder()
{
	return new DotnetBuilder;
}

INITIALIZER(DotnetBuilder)
{
	RegisterBuilder("DOTNET", &CreateDotnetBuilder);
}
