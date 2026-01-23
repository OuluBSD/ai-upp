#include "Builders.h"
#include "UwpTemplates.brc"

 

bool CopyUwpFolder(const String& dst, const String& src)
{
	if(!RealizeDirectory(dst))
		return false;

	FindFile ff(AppendFileName(src, "*"));
	while(ff) {
		String src_path = AppendFileName(src, ff.GetName());
		String dst_path = AppendFileName(dst, ff.GetName());

		if(ff.IsFile()) {
			if(!::SaveFile(dst_path, LoadFile(src_path)))
				return false;
		}
		else
		if(ff.IsFolder() && *ff.GetName() != '.') {
			if(!CopyUwpFolder(dst_path, src_path))
				return false;
		}
		ff.Next();
	}
	return true;
}

String UwpTemplate(const unsigned char *data, int len)
{
	return String((const char *)data, len);
}

Uuid UwpGuidFromString(const String& seed)
{
	String md5 = MD5String(seed);
	Uuid id = ScanUuid(md5);
	if(IsNull(id))
		id = Uuid::Create();
	return id;
}

String UwpGuidString(const String& seed)
{
	return FormatWithDashes(UwpGuidFromString(seed));
}

String ReplaceUwpTokens(String text, const VectorMap<String, String>& tokens)
{
	for(int i = 0; i < tokens.GetCount(); i++) {
		String key = "@" + tokens.GetKey(i) + "@";
		text.Replace(key, tokens[i]);
	}
	return text;
}

String MakeUwpRelativePath(const String& root, const String& path)
{
	String base = UnixPath(NormalizePath(root));
	String full = UnixPath(NormalizePath(path));
	if(!base.IsEmpty()) {
		if(!base.EndsWith("/"))
			base << '/';
		if(full.StartsWith(base))
			return full.Mid(base.GetCount());
	}
	return path;
}

String ToWindowsPath(const String& path)
{
	return NativePath(path);
}

String XmlEscape(const String& s)
{
	StringBuffer out;
	out.Reserve(s.GetCount());
	for(int i = 0; i < s.GetCount(); i++) {
		char c = s[i];
		switch(c) {
		case '&': out << "&amp;"; break;
		case '<': out << "&lt;"; break;
		case '>': out << "&gt;"; break;
		case '"': out << "&quot;"; break;
		case '\'': out << "&apos;"; break;
		default: out.Cat(c); break;
		}
	}
	return String(out);
}

String JoinUwpList(const Vector<String>& items, const char *sep)
{
	String out;
	for(int i = 0; i < items.GetCount(); i++) {
		if(i)
			out << sep;
		out << items[i];
	}
	return out;
}

bool IsCompileSourceFile(const String& path)
{
	String ext = ToLower(GetFileExt(path));
	return findarg(ext, ".c", ".cc", ".cpp", ".cxx", ".icpp") >= 0;
}

bool UwpIsHeaderFile(const String& path)
{
	String ext = ToLower(GetFileExt(path));
	return findarg(ext, ".h", ".hh", ".hpp", ".hxx") >= 0;
}

bool IsCSharpFile(const String& path)
{
	return ToLower(GetFileExt(path)) == ".cs";
}

void CollectUwpProjectFiles(Index<String>& files, const String& source, const String& target)
{
	if(FileExists(source)) {
		files.FindAdd(target);
		return;
	}

	if(!DirectoryExists(source))
		return;

	Vector<String> stack;
	stack.Add(source);
	while(!stack.IsEmpty()) {
		String dir = stack.Top();
		stack.Drop();
		for(FindFile ff(AppendFileName(dir, "*")); ff; ff.Next()) {
			if(ff.IsFolder()) {
				if(*ff.GetName() != '.')
					stack.Add(ff.GetPath());
				continue;
			}
			if(ff.IsFile()) {
				String rel = MakeUwpRelativePath(source, ff.GetPath());
				files.FindAdd(AppendFileName(target, rel));
			}
		}
	}
}

struct UwpAssetMapping : Moveable<UwpAssetMapping> {
	String source;
	String target;
};

String FindUwpRepoRoot(const String& start_dir)
{
	String dir = NormalizePath(start_dir);
	while(!dir.IsEmpty()) {
		if(DirectoryExists(AppendFileName(dir, "share")) && DirectoryExists(AppendFileName(dir, "uppsrc")))
			return dir;
		String parent = GetFileFolder(dir);
		if(parent.IsEmpty() || parent == dir)
			break;
		dir = parent;
	}
	return NormalizePath(start_dir);
}

bool LoadUwpAssetMappings(const String& package, Vector<UwpAssetMapping>& mappings)
{
	String cfg_path = AppendFileName(PackageDirectory(package), "UwpAssets.json");
	if(!FileExists(cfg_path))
		return false;

	Value json;
	try {
		json = ParseJSON(LoadFile(cfg_path));
	}
	catch(CParser::Error& e) {
		PutConsole("UWP: failed to parse " + cfg_path + ": " + String(e));
		return false;
	}

	if(!json.Is<ValueMap>()) {
		PutConsole("UWP: invalid " + cfg_path + ": root must be object");
		return false;
	}

	ValueMap root = json;
	Value items = root.Get("mappings", Value());
	if(!items.Is<ValueArray>()) {
		PutConsole("UWP: invalid " + cfg_path + ": mappings must be array");
		return false;
	}

	ValueArray arr = items;
	for(int i = 0; i < arr.GetCount(); i++) {
		if(!arr[i].Is<ValueMap>())
			continue;
		ValueMap item = arr[i];
		String source = item.Get("source", String());
		String target = item.Get("target", String());
		if(source.IsEmpty() || target.IsEmpty())
			continue;
		UwpAssetMapping& mapping = mappings.Add();
		mapping.source = source;
		mapping.target = target;
	}

	return !mappings.IsEmpty();
}

String MakeItemList(const Index<String>& files, const String& root, const String& tag)
{
	StringBuffer out;
	for(int i = 0; i < files.GetCount(); i++) {
		String rel = XmlEscape(ToWindowsPath(MakeUwpRelativePath(root, files[i])));
		if(tag == "ClCompile" && ToLower(GetFileExt(files[i])) == ".c") {
			out << "    <" << tag << " Include=\"" << rel << "\">\n"
			    << "      <CompileAs>CompileAsC</CompileAs>\n"
			    << "      <CompileAsWinRT>false</CompileAsWinRT>\n"
			    << "    </" << tag << ">\n";
		}
		else {
			out << "    <" << tag << " Include=\"" << rel << "\" />\n";
		}
	}
	return String(out);
}

String MakeFilteredItemList(const Index<String>& files, const String& root, const String& tag, const String& filter)
{
	StringBuffer out;
	for(int i = 0; i < files.GetCount(); i++) {
		String rel = XmlEscape(ToWindowsPath(MakeUwpRelativePath(root, files[i])));
		out << "    <" << tag << " Include=\"" << rel << "\">\n"
		    << "      <Filter>" << XmlEscape(filter) << "</Filter>\n"
		    << "    </" << tag << ">\n";
	}
	return String(out);
}

String MakeContentItemList(const Index<String>& files, const String& root)
{
	StringBuffer out;
	for(int i = 0; i < files.GetCount(); i++) {
		String rel = XmlEscape(ToWindowsPath(MakeUwpRelativePath(root, files[i])));
		out << "    <Content Include=\"" << rel << "\">\n"
		    << "      <DeploymentContent>true</DeploymentContent>\n"
		    << "      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>\n"
		    << "    </Content>\n";
	}
	return String(out);
}

String MakeFilteredContentItemList(const Index<String>& files, const String& root, const String& filter)
{
	StringBuffer out;
	for(int i = 0; i < files.GetCount(); i++) {
		String rel = XmlEscape(ToWindowsPath(MakeUwpRelativePath(root, files[i])));
		out << "    <Content Include=\"" << rel << "\">\n"
		    << "      <DeploymentContent>true</DeploymentContent>\n"
		    << "      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>\n"
		    << "      <Filter>" << XmlEscape(filter) << "</Filter>\n"
		    << "    </Content>\n";
	}
	return String(out);
}

String UwpProjectNameFromPackage(const String& package)
{
	String name = package;
	name.Replace("\\", "_");
	name.Replace("/", "_");
	return name;
}

String UwpProjectFileFromPackage(const String& package)
{
	return UwpProjectNameFromPackage(package) + ".vcxproj";
}

void WriteUwpAssets(const String& assets_dir)
{
	static const char *kStoreLogoBase64 =
		"iVBORw0KGgoAAAANSUhEUgAAADIAAAAyCAYAAAAeP4ixAAAAAXNSR0IArs4c6QAAAARn"
		"QU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAgSURBVGhD7cEBAQAAAIIg"
		"/69uSEAAAAAAAAAAAABwqAYnQgABPpNbOAAAAABJRU5ErkJggg==";
	static const char *kSquare150Base64 =
		"iVBORw0KGgoAAAANSUhEUgAAAJYAAACWCAYAAAA8AXHiAAAAAXNSR0IArs4c6QAAAARn"
		"QU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAABuSURBVHhe7cExAQAAAMKg"
		"9U9tCy8gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAbmpgNQABECgQtgAAAABJRU5ErkJggg==";
	static const char *kSquare44Base64 =
		"iVBORw0KGgoAAAANSUhEUgAAACwAAAAsCAYAAAAehFoBAAAAAXNSR0IArs4c6QAAAARn"
		"QU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAAAfSURBVFhH7cExAQAAAMKg"
		"9U9tBn8gAAAAAAAAALjUAB5sAAHaZvU3AAAAAElFTkSuQmCC";
	static const char *kWideBase64 =
		"iVBORw0KGgoAAAANSUhEUgAAATYAAACWCAYAAABU16TzAAAAAXNSR0IArs4c6QAAAARn"
		"QU1BAACxjwv8YQUAAAAJcEhZcwAADsMAAA7DAcdvqGQAAADLSURBVHhe7cExAQAAAMKg"
		"9U9tDB8gAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA7mrXRAABseYNLQAA"
		"AABJRU5ErkJggg==";
	String store_png = Base64Decode(kStoreLogoBase64);
	String square150_png = Base64Decode(kSquare150Base64);
	String square44_png = Base64Decode(kSquare44Base64);
	String wide_png = Base64Decode(kWideBase64);
	RealizeDirectory(assets_dir);
	SaveFile(AppendFileName(assets_dir, "StoreLogo.png"), store_png);
	SaveFile(AppendFileName(assets_dir, "Square150x150Logo.png"), square150_png);
	SaveFile(AppendFileName(assets_dir, "Square44x44Logo.png"), square44_png);
	SaveFile(AppendFileName(assets_dir, "Wide310x150Logo.png"), wide_png);
}

String GetUwpRelativePath(const String& package, const String& path)
{
	String base = UnixPath(NormalizePath(PackageDirectory(package)));
	String full = UnixPath(NormalizePath(path));

	if(!base.IsEmpty()) {
		if(!base.EndsWith("/"))
			base << '/';
		if(full.StartsWith(base))
			return full.Mid(base.GetCount());
	}
	return GetFileName(path);
}

bool StageUwpFile(const String& source, const String& target, bool& updated)
{
	updated = false;
	if(!FileExists(source)) {
		if(!DirectoryExists(source)) {
			PutConsole(Format("UWP: missing source file %s", source));
			return false;
		}

		if(DirectoryExists(target) && GetFileTime(target) >= GetFileTime(source))
			return true;

		updated = true;
		if(FileExists(target))
			DeleteFile(target);
		if(DirectoryExists(target))
			DeleteFolderDeep(target);

		String directory = GetFileDirectory(target);
		if(!RealizeDirectory(directory)) {
			PutConsole(Format("UWP: failed to create directory %s", directory));
			return false;
		}

#ifdef PLATFORM_POSIX
		if(symlink(source, target) == 0)
			return true;
#endif

		if(!CopyUwpFolder(target, source)) {
			PutConsole(Format("UWP: failed to copy %s to %s", source, target));
			return false;
		}
		return true;
	}

	if(FileExists(target) && GetFileTime(target) >= GetFileTime(source))
		return true;

	if(DirectoryExists(target))
		DeleteFolderDeep(target);

	String directory = GetFileDirectory(target);
	if(!RealizeDirectory(directory)) {
		PutConsole(Format("UWP: failed to create directory %s", directory));
		return false;
	}

	updated = true;
	DeleteFile(target);

#ifdef PLATFORM_POSIX
	if(symlink(source, target) == 0)
		return true;
#endif

	if(!::SaveFile(target, LoadFile(source))) {
		PutConsole(Format("UWP: failed to copy %s to %s", source, target));
		return false;
	}

	return true;
}

String MakeUwpTargetPath(const String& outdir, const String& package, const String& source)
{
	String rel = NativePath(GetUwpRelativePath(package, source));
	return AppendFileName(outdir, AppendFileName(package, rel));
}

UwpBuilder::UwpProjectData& UwpBuilder::GetProjectData(const String& package)
{
	int idx = project_map.FindAdd(package);
	if(project_map[idx].root.IsEmpty())
		project_map[idx].root = AppendFileName(outdir, package);
	return project_map[idx];
}

void UwpBuilder::AddFlags(Index<String>& cfg)
{
	MscBuilder::AddFlags(cfg);
	cfg.FindAdd("UWP");
	if(cfg.Find("ARM") < 0 && cfg.Find("MIPS") < 0 && !IsMsc64()) {
		cfg.FindAdd("WIN64");
		cfg.FindAdd("MSC22X64");
	}
}

bool UwpBuilder::BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>&,
	String& linkoptions, const Vector<String>& all_uses, const Vector<String>&, int)
{
	if(!uwp_started) {
		project_files.Clear();
		project_map.Clear();
		uwp_started = true;
	}

	int time = msecs();
	Package pkg;
	pkg.Load(PackageFile(package));
	String packagedir = PackageDirectory(package);
	GetProjectData(package);
	if(package == mainpackage) {
		main_uses_gui = FindIndex(all_uses, "CtrlLib") >= 0 || FindIndex(all_uses, "CtrlCore") >= 0;
	}

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
		if(srcfile.IsEmpty()) {
			PutConsole("UWP: CustomStep returned empty for " + (String)pkg[i]);
			error = true;
		}

		for(int j = 0; j < srcfile.GetCount(); j++) {
			if(!IdeIsBuilding())
				return false;

			bool updated = false;
			String target = MakeUwpTargetPath(outdir, package, srcfile[j]);
			// PutConsole("UWP: staging " + srcfile[j] + " to " + target);
			CollectUwpProjectFiles(project_files, srcfile[j], target);
			if(!StageUwpFile(srcfile[j], target, updated))
				error = true;
			else if(updated)
				staged++;
		}
	}
	
	// Stage all headers from package directory to ensure internal headers are present
	Vector<String> subdirs;
	subdirs.Add("");
	while(!subdirs.IsEmpty()) {
		String subdir = subdirs.Top();
		subdirs.Drop();
		String dir = AppendFileName(packagedir, subdir);
		FindFile ff(AppendFileName(dir, "*"));
		while(ff) {
			if(ff.IsFile()) {
				String ext = ToLower(GetFileExt(ff.GetName()));
				if(findarg(ext, ".h", ".hpp", ".hxx", ".hh", ".i", ".t") >= 0) {
					bool updated = false;
					String source = ff.GetPath();
					String rel = subdir.IsEmpty() ? ff.GetName() : AppendFileName(subdir, ff.GetName());
					String target = AppendFileName(AppendFileName(outdir, package), rel);
					CollectUwpProjectFiles(project_files, source, target);
					if(!StageUwpFile(source, target, updated))
						error = true;
					else if(updated)
						staged++;
				}
			}
			else
			if(ff.IsFolder() && *ff.GetName() != '.') {
				subdirs.Add(subdir.IsEmpty() ? ff.GetName() : AppendFileName(subdir, ff.GetName()));
			}
			ff.Next();
		}
	}

	Vector<UwpAssetMapping> asset_mappings;
	if(LoadUwpAssetMappings(package, asset_mappings)) {
		String repo_root = FindUwpRepoRoot(packagedir);
		UwpProjectData& data = project_map.Get(package);
		for(int i = 0; i < asset_mappings.GetCount(); i++) {
			const UwpAssetMapping& mapping = asset_mappings[i];
			String source = mapping.source;
			if(!IsFullPath(source))
				source = AppendFileName(repo_root, source);
			String target = AppendFileName(AppendFileName(outdir, package), NativePath(mapping.target));

			bool updated = false;
			Index<String> content_files;
			CollectUwpProjectFiles(content_files, source, target);
			for(int j = 0; j < content_files.GetCount(); j++) {
				const String& item = content_files[j];
				data.content.FindAdd(item);
				project_files.FindAdd(item);
			}
			if(!StageUwpFile(source, target, updated))
				error = true;
			else if(updated)
				staged++;
		}
	}
	
	PutConsole("UWP: package " + package + " staged, project_files count: " + FormatInt(project_files.GetCount()));

	if(staged > 0)
		PutConsole(String().Cat() << staged << " file(s) staged in " << GetPrintTime(time));

	linkfile.Add(outdir);
	linkoptions << ' ' << Gather(pkg.link, config.GetKeys());
	return !error;
}

String UwpBuilder::GetMsBuildPath() const
{
	String vswhere = "C:\\Program Files (x86)\\Microsoft Visual Studio\\Installer\\vswhere.exe";
	if(!FileExists(vswhere)) {
		String p86 = GetEnv("ProgramFiles(x86)");
		if(p86.GetCount())
			vswhere = AppendFileName(p86, "Microsoft Visual Studio\\Installer\\vswhere.exe");
	}
	
	if(FileExists(vswhere)) {
		String out = HostSys(GetPathQ(vswhere) + " -latest -requires Microsoft.Component.MSBuild -find MSBuild\\**\\Bin\\MSBuild.exe");
		out = TrimBoth(out);
		if(out.GetCount())
			return out;
	}
	return "msbuild";
}

bool UwpBuilder::Link(const Vector<String>&, const String&, bool)
{
	PutConsole("DEBUG: Link started, project_files count: " + FormatInt(project_files.GetCount()));
	if(project_files.IsEmpty()) {
		PutConsole("UWP: no sources staged.");
		uwp_started = false;
		return true;
	}

	String solution_dir = outdir;
	String solution_name = GetFileTitle(mainpackage);
	if(IsNull(solution_name))
		solution_name = "UwpApp";

	String project_type_guid_cpp = "BC8A1FFA-BEE3-4634-8014-F334798102B3";
	String project_type_guid_cs = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC";
	
	String target_version = "10.0.19041.0";
	String min_version = "10.0.17763.0";
	String entry_point = "Upp.AppView"; // C++/WinRT IFrameworkView class in Upp namespace
	String uwp_csharp_framework = "uap10.0.19041";

	// Auto-increment package version based on installed version
	// Query installed package version and increment the revision
	int build_number = 0;
	String cmd;
	cmd << "powershell -NoProfile -Command \"(Get-AppxPackage -Name '" << solution_name << "').Version\"";
	String installed_version;
	if(Sys(cmd, installed_version) == 0) {
		installed_version = TrimBoth(installed_version);
		if(!IsNull(installed_version)) {
			// Parse version like "1.0.0.5" and get the last number
			Vector<String> parts = Split(installed_version, '.');
			if(parts.GetCount() >= 4) {
				build_number = ScanInt(parts[3]);
			}
		}
	}
	build_number++;
	String package_version = Format("1.0.0.%d", build_number);
	PutConsole("UWP: Package version: " + package_version + (IsNull(installed_version) ? " (new)" : " (was: " + installed_version + ")"));

	Index<String> clcompile;
	Index<String> clincludes;
	Index<String> none;
	Index<String> cscompile;
	bool has_cpp = false;
	bool has_cs = false;

	for(int i = 0; i < project_files.GetCount(); i++) {
		const String& path = project_files[i];
		if (i < 10) PutConsole("DEBUG: project_file[" + FormatInt(i) + "] = " + path);
		if(IsCSharpFile(path)) {
			cscompile.FindAdd(path);
			has_cs = true;
		}
		else
		if(IsCompileSourceFile(path)) {
			clcompile.FindAdd(path);
			has_cpp = true;
		}
		else
		if(UwpIsHeaderFile(path))
			clincludes.FindAdd(path);
		else
			none.FindAdd(path);
	}

	bool use_csharp = has_cs && !has_cpp;
	if(use_csharp)
		entry_point = solution_name + ".App";

	String manifest_path = AppendFileName(solution_dir, "Package.appxmanifest");

	String assets_dir = AppendFileName(solution_dir, "Assets");
	WriteUwpAssets(assets_dir);

	Index<String> assets;
	assets.Add(AppendFileName(assets_dir, "StoreLogo.png"));
	assets.Add(AppendFileName(assets_dir, "Square150x150Logo.png"));
	assets.Add(AppendFileName(assets_dir, "Square44x44Logo.png"));
	assets.Add(AppendFileName(assets_dir, "Wide310x150Logo.png"));

	Vector<String> inc;
	for(int i = 0; i < include.GetCount(); i++)
		inc.Add(include[i]);
	inc.Add(outdir);
	String include_dirs = JoinUwpList(inc, ";");
	if(include_dirs.GetCount())
		include_dirs << ";%(AdditionalIncludeDirectories)";
	else
		include_dirs = "%(AdditionalIncludeDirectories)";

	Vector<String> defs;
	for(int i = 0; i < config.GetCount(); i++)
		defs.Add("flag" + config[i]);
	if(main_conf)
		defs.Add("MAIN_CONF");
	String defines = JoinUwpList(defs, ";");
	if(defines.GetCount())
		defines << ";%(PreprocessorDefinitions)";
	else
		defines = "%(PreprocessorDefinitions)";

	Vector<String> project_order;
	VectorMap<String, String> project_guids;
	VectorMap<String, String> project_names;
	VectorMap<String, String> project_files_map;
	VectorMap<String, VectorMap<String, String>> project_tokens;

	if(use_csharp) {
		String project_guid = UwpGuidString("UWPProject:" + solution_name);
		String project_file = solution_name + ".csproj";
		String project_type_guid = project_type_guid_cs;

		VectorMap<String, String> sln_tokens;
		sln_tokens.Add("PROJECT_NAME", solution_name);
		sln_tokens.Add("PROJECT_GUID", project_guid);
		sln_tokens.Add("PROJECT_TYPE_GUID", project_type_guid);
		sln_tokens.Add("PROJECT_FILE", project_file);

		String sln = ReplaceUwpTokens(UwpTemplate(uwp_sln_tpl, uwp_sln_tpl_length), sln_tokens);
		SaveFile(AppendFileName(solution_dir, solution_name + ".sln"), sln);

		VectorMap<String, String> proj_tokens;
		proj_tokens.Add("PROJECT_GUID", XmlEscape(project_guid));
		proj_tokens.Add("ROOT_NAMESPACE", XmlEscape(solution_name));
		proj_tokens.Add("PROJECT_NAME", XmlEscape(solution_name));
		proj_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
		proj_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));
		proj_tokens.Add("TARGET_FRAMEWORK", XmlEscape(uwp_csharp_framework));
		proj_tokens.Add("CSCOMPILE", String());
		proj_tokens.Add("NONEITEMS", MakeItemList(none, solution_dir, "None"));
		proj_tokens.Add("CONTENT_ITEMS", MakeContentItemList(assets, solution_dir));

		String csproj = ReplaceUwpTokens(UwpTemplate(uwp_csproj_tpl, uwp_csproj_tpl_length), proj_tokens);
		SaveFile(AppendFileName(solution_dir, solution_name + ".csproj"), csproj);

		VectorMap<String, String> manifest_tokens;
		manifest_tokens.Add("PACKAGE_NAME", XmlEscape(solution_name));
		manifest_tokens.Add("PROJECT_NAME", XmlEscape(solution_name));
		manifest_tokens.Add("PACKAGE_VERSION", XmlEscape(package_version));
		manifest_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
		manifest_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));
		manifest_tokens.Add("ENTRY_POINT", XmlEscape(entry_point));
		manifest_tokens.Add("PHONE_PRODUCT_ID", XmlEscape(UwpGuidString("PhoneProduct:" + solution_name)));
		manifest_tokens.Add("PHONE_PUBLISHER_ID", XmlEscape(UwpGuidString("PhonePublisher:" + solution_name)));

		String manifest = ReplaceUwpTokens(UwpTemplate(uwp_appxmanifest_tpl, uwp_appxmanifest_tpl_length), manifest_tokens);
		SaveFile(manifest_path, manifest);
	}
	else {
		struct PackageRoot : Moveable<PackageRoot> {
			String package;
			String root;
			int    length = 0;
		};

		Vector<PackageRoot> roots;
		for(int i = 0; i < project_map.GetCount(); i++) {
			PackageRoot root;
			root.package = project_map.GetKey(i);
			root.root = UnixPath(NormalizePath(project_map[i].root));
			if(!root.root.EndsWith("/"))
				root.root << '/';
			root.length = root.root.GetCount();
			roots.Add(root);
		}
		Sort(roots, [](const PackageRoot& a, const PackageRoot& b) { return a.length > b.length; });

		for(int i = 0; i < project_files.GetCount(); i++) {
			String path = UnixPath(NormalizePath(project_files[i]));
			String package;
			for(int r = 0; r < roots.GetCount(); r++) {
				if(path.StartsWith(roots[r].root)) {
					package = roots[r].package;
					break;
				}
			}
			if(IsNull(package))
				continue;
			UwpProjectData& data = project_map.Get(package);
			if(data.content.Find(project_files[i]) >= 0)
				data.content.FindAdd(project_files[i]);
			else
			if(IsCompileSourceFile(project_files[i]))
				data.clcompile.FindAdd(project_files[i]);
			else
			if(UwpIsHeaderFile(project_files[i]))
				data.clincludes.FindAdd(project_files[i]);
			else
				data.none.FindAdd(project_files[i]);
		}

		if(HasFlag("UWP_SMOKETEST")) {
			for(int i = 0; i < project_map.GetCount(); i++) {
				String pkg = project_map.GetKey(i);
				if(pkg != mainpackage) {
					project_map[i].clcompile.Clear();
					project_map[i].clincludes.Clear();
					project_map[i].none.Clear();
				}
			}
			int main_idx = project_map.Find(mainpackage);
			if(main_idx >= 0 && !project_map[main_idx].clcompile.IsEmpty()) {
				String preferred = project_map[main_idx].clcompile[0];
				Index<String> only;
				only.Add(preferred);
				project_map[main_idx].clcompile = pick(only);
				PutConsole("UWP: smoketest enabled, compiling only " + preferred);
			}
		}

		Vector<String> packages;
		if(HasFlag("UWP_SMOKETEST")) {
			if(project_map.Find(mainpackage) >= 0)
				packages.Add(mainpackage);
		}
		else {
			if(project_map.Find(mainpackage) >= 0)
				packages.Add(mainpackage);
			for(int i = 0; i < project_map.GetCount(); i++) {
				String pkg = project_map.GetKey(i);
				if(FindIndex(packages, pkg) < 0)
					packages.Add(pkg);
			}
		}
		project_order = clone(packages);
		for(int i = 0; i < packages.GetCount(); i++) {
			String pkg = packages[i];
			project_guids.Add(pkg, UwpGuidString("UWPProject:" + pkg));
			project_names.Add(pkg, UwpProjectNameFromPackage(pkg));
			project_files_map.Add(pkg, UwpProjectFileFromPackage(pkg));
		}

		StringBuffer sln;
		sln << "Microsoft Visual Studio Solution File, Format Version 12.00\n"
		    << "# Visual Studio Version 16\n"
		    << "VisualStudioVersion = 16.0.31019.35\n"
		    << "MinimumVisualStudioVersion = 10.0.40219.1\n";
		for(int i = 0; i < packages.GetCount(); i++) {
			String pkg = packages[i];
			sln << "Project(\"{" << project_type_guid_cpp << "}\") = \""
			    << project_names.Get(pkg) << "\", \""
			    << project_files_map.Get(pkg) << "\", \"{"
			    << project_guids.Get(pkg) << "}\"\n"
			    << "EndProject\n";
		}
		sln << "Global\n"
		    << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution\n"
		    << "\t\tDebug|x64 = Debug|x64\n"
		    << "\t\tRelease|x64 = Release|x64\n"
		    << "\tEndGlobalSection\n"
		    << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution\n";
		for(int i = 0; i < packages.GetCount(); i++) {
			String pkg = packages[i];
			String guid = project_guids.Get(pkg);
			sln << "\t\t{" << guid << "}.Debug|x64.ActiveCfg = Debug|x64\n"
			    << "\t\t{" << guid << "}.Debug|x64.Build.0 = Debug|x64\n"
			    << "\t\t{" << guid << "}.Release|x64.ActiveCfg = Release|x64\n"
			    << "\t\t{" << guid << "}.Release|x64.Build.0 = Release|x64\n";
			if(pkg == mainpackage && !HasFlag("UWP_SMOKETEST")) {
				sln << "\t\t{" << guid << "}.Debug|x64.Deploy.0 = Debug|x64\n"
				    << "\t\t{" << guid << "}.Release|x64.Deploy.0 = Release|x64\n";
			}
		}
		sln << "\tEndGlobalSection\n"
		    << "\tGlobalSection(SolutionProperties) = preSolution\n"
		    << "\t\tHideSolutionNode = FALSE\n"
		    << "\tEndGlobalSection\n"
		    << "EndGlobal\n";
		SaveFile(AppendFileName(solution_dir, solution_name + ".sln"), sln);

		VectorMap<String, String> manifest_tokens;
		manifest_tokens.Add("PACKAGE_NAME", XmlEscape(solution_name));
		manifest_tokens.Add("PROJECT_NAME", XmlEscape(solution_name));
		manifest_tokens.Add("PACKAGE_VERSION", XmlEscape(package_version));
		manifest_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
		manifest_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));
		manifest_tokens.Add("ENTRY_POINT", XmlEscape(entry_point));
		manifest_tokens.Add("PHONE_PRODUCT_ID", XmlEscape(UwpGuidString("PhoneProduct:" + solution_name)));
		manifest_tokens.Add("PHONE_PUBLISHER_ID", XmlEscape(UwpGuidString("PhonePublisher:" + solution_name)));
		String manifest = ReplaceUwpTokens(UwpTemplate(uwp_appxmanifest_tpl, uwp_appxmanifest_tpl_length), manifest_tokens);
		SaveFile(manifest_path, manifest);

		for(int i = 0; i < packages.GetCount(); i++) {
			String pkg = packages[i];
			bool is_main = pkg == mainpackage;
			bool is_app = is_main && !HasFlag("UWP_SMOKETEST");
			UwpProjectData& data = project_map.Get(pkg);

			String filter_sources_guid = UwpGuidString("UWPFilter:Source Files:" + pkg);
			String filter_headers_guid = UwpGuidString("UWPFilter:Header Files:" + pkg);
			String filter_resources_guid = UwpGuidString("UWPFilter:Resource Files:" + pkg);
			String filter_content_guid = UwpGuidString("UWPFilter:Content Files:" + pkg);

			StringBuffer filters;
			filters << "    <Filter Include=\"Source Files\">\n"
			        << "      <UniqueIdentifier>{" << filter_sources_guid << "}</UniqueIdentifier>\n"
			        << "    </Filter>\n"
			        << "    <Filter Include=\"Header Files\">\n"
			        << "      <UniqueIdentifier>{" << filter_headers_guid << "}</UniqueIdentifier>\n"
			        << "    </Filter>\n"
			        << "    <Filter Include=\"Resource Files\">\n"
			        << "      <UniqueIdentifier>{" << filter_resources_guid << "}</UniqueIdentifier>\n"
			        << "    </Filter>\n"
			        << "    <Filter Include=\"Content Files\">\n"
			        << "      <UniqueIdentifier>{" << filter_content_guid << "}</UniqueIdentifier>\n"
			        << "    </Filter>\n";

			String project_refs;
			if(is_app) {
				StringBuffer refs;
				refs << "  <ItemGroup>\n";
				for(int r = 0; r < packages.GetCount(); r++) {
					String ref_pkg = packages[r];
					if(ref_pkg == mainpackage)
						continue;
					refs << "    <ProjectReference Include=\""
					     << XmlEscape(project_files_map.Get(ref_pkg)) << "\">\n"
					     << "      <Project>{" << project_guids.Get(ref_pkg) << "}</Project>\n"
					     << "      <ReferenceOutputAssembly>false</ReferenceOutputAssembly>\n"
					     << "      <LinkLibraryDependencies>true</LinkLibraryDependencies>\n"
					    << "    </ProjectReference>\n";
				}
				refs << "  </ItemGroup>\n";
				project_refs = String(refs);
			}

			String appx_item;
			if(is_app) {
				StringBuffer item;
				item << "  <ItemGroup>\n"
				     << "    <AppxManifest Include=\"Package.appxmanifest\" />\n"
				     << "  </ItemGroup>\n";
				appx_item = String(item);
			}

			String appx_package;
			if(!is_app) {
				StringBuffer pkg;
				pkg << "    <AppxPackage>false</AppxPackage>\n"
				    << "    <GenerateAppxPackageOnBuild>false</GenerateAppxPackageOnBuild>\n"
				    << "    <GenerateWindowsMetadata>false</GenerateWindowsMetadata>\n"
				    << "    <WinMDOutput>false</WinMDOutput>\n";
				appx_package = String(pkg);
			}

			String project_defines = defines;
			if(!is_main) {
				Vector<String> def_parts = Split(defines, ';');
				Vector<String> filtered;
				for(int d = 0; d < def_parts.GetCount(); d++) {
					const String& part = def_parts[d];
					if(part.IsEmpty() || part == "flagMAIN")
						continue;
					filtered.Add(part);
				}
				project_defines = JoinUwpList(filtered, ";");
			}

			String link_settings;
			if(is_app) {
				StringBuffer link;
				link << "    <Link>\n"
				     << "      <SubSystem>" << (main_uses_gui ? "Windows" : "Console") << "</SubSystem>\n"
				     << "    </Link>\n";
				link_settings = String(link);
			}

			VectorMap<String, String> proj_tokens;
			proj_tokens.Add("PROJECT_GUID", XmlEscape(project_guids.Get(pkg)));
			proj_tokens.Add("ROOT_NAMESPACE", XmlEscape(project_names.Get(pkg)));
			proj_tokens.Add("PROJECT_NAME", XmlEscape(project_names.Get(pkg)));
			proj_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
			proj_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));
			proj_tokens.Add("PLATFORM_TOOLSET", XmlEscape(String()));
			proj_tokens.Add("INCLUDE_DIRS", XmlEscape(include_dirs));
			proj_tokens.Add("DEFINES", XmlEscape(project_defines));
			proj_tokens.Add("CONFIGURATION_TYPE", XmlEscape(is_app ? "Application" : "StaticLibrary"));
			proj_tokens.Add("CLCOMPILE", MakeItemList(data.clcompile, solution_dir, "ClCompile"));
			proj_tokens.Add("CLINCLUDE", MakeItemList(data.clincludes, solution_dir, "ClInclude"));
			proj_tokens.Add("NONEITEMS", MakeItemList(data.none, solution_dir, "None"));
			Index<String> content_items;
			if(is_app) {
				for(int a = 0; a < assets.GetCount(); a++)
					content_items.FindAdd(assets[a]);
			}
			for(int c = 0; c < data.content.GetCount(); c++)
				content_items.FindAdd(data.content[c]);
			proj_tokens.Add("CONTENT_ITEMS", MakeContentItemList(content_items, solution_dir));
			proj_tokens.Add("PROJECT_REFERENCES", project_refs);
			proj_tokens.Add("APPX_MANIFEST_ITEM", appx_item);
			proj_tokens.Add("APPX_PACKAGE", appx_package);
			proj_tokens.Add("LINK_SETTINGS", link_settings);
			project_tokens.GetAdd(pkg) = pick(proj_tokens);

			VectorMap<String, String> filter_tokens;
			filter_tokens.Add("FILTERS", String(filters));
			filter_tokens.Add("FILTER_CLCOMPILE", MakeFilteredItemList(data.clcompile, solution_dir, "ClCompile", "Source Files"));
			filter_tokens.Add("FILTER_CLINCLUDE", MakeFilteredItemList(data.clincludes, solution_dir, "ClInclude", "Header Files"));
			Index<String> none_filters;
			for(int n = 0; n < data.none.GetCount(); n++)
				none_filters.FindAdd(data.none[n]);
			if(is_main)
				none_filters.FindAdd(manifest_path);
			filter_tokens.Add("FILTER_NONE", MakeFilteredItemList(none_filters, solution_dir, "None", "Resource Files"));
			Index<String> filter_content_items;
			if(is_main) {
				for(int a = 0; a < assets.GetCount(); a++)
					filter_content_items.FindAdd(assets[a]);
			}
			for(int c = 0; c < data.content.GetCount(); c++)
				filter_content_items.FindAdd(data.content[c]);
			filter_tokens.Add("FILTER_CONTENT", MakeFilteredContentItemList(filter_content_items, solution_dir, "Content Files"));

			String vcxfilters = ReplaceUwpTokens(UwpTemplate(uwp_filters_tpl, uwp_filters_tpl_length), filter_tokens);
			SaveFile(AppendFileName(solution_dir, project_names.Get(pkg) + ".vcxproj.filters"), vcxfilters);

		}
	}

#ifdef PLATFORM_WIN32
	String conf = HasFlag("DEBUG") ? "Debug" : "Release";
	String sln_path = AppendFileName(solution_dir, solution_name + ".sln");
	String msbuild = GetMsBuildPath();
	cmd.Clear();
	cmd << GetPathQ(msbuild) << " " << GetPathQ(sln_path) << " /p:Configuration=" << conf << " /p:Platform=x64";
	PutConsole("UWP: building solution with msbuild...");
	
	Vector<String> toolsets;
	if(!use_csharp) {
		toolsets.Add("v143");
		toolsets.Add("v142");
		if(HasFlag("MSC19") || HasFlag("MSC19X64")) {
			toolsets.Remove(0);
			toolsets.Add("v143");
		}
	} else {
		toolsets.Add(""); // Dummy for C# loop
	}

	for(int t = 0; t < toolsets.GetCount(); t++) {
		if(!use_csharp) {
			String toolset = toolsets[t];
			for(int i = 0; i < project_order.GetCount(); i++) {
				String pkg = project_order[i];
				int idx = project_tokens.Find(pkg);
				if(idx < 0)
					continue;
				project_tokens[idx].GetAdd("PLATFORM_TOOLSET") = XmlEscape(toolset);
				String vcxproj = ReplaceUwpTokens(UwpTemplate(uwp_vcxproj_tpl, uwp_vcxproj_tpl_length), project_tokens[idx]);
				SaveFile(AppendFileName(solution_dir, project_files_map.Get(pkg)), vcxproj);
			}
		}

		StringStream out;
		int exitcode = Execute(cmd, out);
		PutConsole(out.GetResult());
		
		if(exitcode == 0) {
			PutConsole(Format("UWP: wrote Visual Studio project files to %s", solution_dir));
			uwp_started = false;
			return true;
		}
		
		if(!use_csharp && t < toolsets.GetCount() - 1 && out.GetResult().Find("MSB8020") >= 0) {
			PutConsole("UWP: Toolset " + toolsets[t] + " not found. Retrying with " + toolsets[t+1] + "...");
			continue;
		}
		
		PutConsole("UWP: msbuild failed.");
		uwp_started = false;
		return false;
	}
#else
	PutConsole("UWP: build skipped (requires Windows MSBuild).");
#endif

	PutConsole(Format("UWP: wrote Visual Studio project files to %s", solution_dir));
	uwp_started = false;
	return true;
}

static Builder *CreateUwpBuilder()
{
	return new UwpBuilder;
}

INITIALIZER(UwpBuilder)
{
	RegisterBuilder("UWP", &CreateUwpBuilder);
}

Index<String> UwpBuilder::project_files;
bool          UwpBuilder::uwp_started = false;
VectorMap<String, UwpBuilder::UwpProjectData> UwpBuilder::project_map;

