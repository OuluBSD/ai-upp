#include "Builders.h"
#include "UwpTemplates.brc"

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
	String target_framework = "net6.0";

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
	proj_tokens.Add("CSCOMPILE", MakeItemList(cscompile, solution_dir, "Compile"));
	proj_tokens.Add("NONEITEMS", MakeItemList(none, solution_dir, "None"));

	String csproj = ReplaceUwpTokens(UwpTemplate(dotnet_csproj_tpl, dotnet_csproj_tpl_length), proj_tokens);
	SaveFile(AppendFileName(solution_dir, project_name + ".csproj"), csproj);

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
