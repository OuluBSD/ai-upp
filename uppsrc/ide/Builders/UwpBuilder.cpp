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
	return GetFileName(path);
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

String MakeItemList(const Index<String>& files, const String& root, const String& tag)
{
	StringBuffer out;
	for(int i = 0; i < files.GetCount(); i++) {
		String rel = XmlEscape(ToWindowsPath(MakeUwpRelativePath(root, files[i])));
		out << "    <" << tag << " Include=\"" << rel << "\" />\n";
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
		    << "      <Filter>" << XmlEscape(filter) << "</Filter>\n"
		    << "    </Content>\n";
	}
	return String(out);
}

void WriteUwpAssets(const String& assets_dir)
{
	static const char *kPngBase64 =
		"iVBORw0KGgoAAAANSUhEUgAAAAEAAAABCAQAAAC1HAwCAAAAC0lEQVR42mP8/"
		"x8AAwMBApU3a6QAAAAASUVORK5CYII=";
	String png = Base64Decode(kPngBase64);
	RealizeDirectory(assets_dir);
	SaveFile(AppendFileName(assets_dir, "StoreLogo.png"), png);
	SaveFile(AppendFileName(assets_dir, "Square150x150Logo.png"), png);
	SaveFile(AppendFileName(assets_dir, "Square44x44Logo.png"), png);
	SaveFile(AppendFileName(assets_dir, "Wide310x150Logo.png"), png);
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

void UwpBuilder::AddFlags(Index<String>& cfg)
{
	MscBuilder::AddFlags(cfg);
	cfg.FindAdd("UWP");
}

bool UwpBuilder::BuildPackage(const String& package, Vector<String>& linkfile, Vector<String>&,
	String& linkoptions, const Vector<String>&, const Vector<String>&, int)
{
	if(!uwp_started) {
		project_files.Clear();
		uwp_started = true;
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
			String target = MakeUwpTargetPath(outdir, package, srcfile[j]);
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

bool UwpBuilder::Link(const Vector<String>&, const String&, bool)
{
	if(project_files.IsEmpty()) {
		PutConsole("UWP: no sources staged.");
		uwp_started = false;
		return true;
	}

	String solution_dir = outdir;
	String project_name = GetFileTitle(mainpackage);
	if(IsNull(project_name))
		project_name = "UwpApp";

	String project_guid = UwpGuidString("UWPProject:" + project_name);
	String project_type_guid_cpp = "BC8A1FFA-BEE3-4634-8014-F334798102B3";
	String project_type_guid_cs = "FAE04EC0-301F-11D3-BF4B-00C04F79EFBC";
	String toolset = "v142";
	String target_version = "10.0.19041.0";
	String min_version = "10.0.17763.0";
	String entry_point = "App";
	String uwp_csharp_framework = "uap10.0.19041";

	Index<String> clcompile;
	Index<String> clincludes;
	Index<String> none;
	Index<String> cscompile;
	bool has_cpp = false;
	bool has_cs = false;

	for(int i = 0; i < project_files.GetCount(); i++) {
		const String& path = project_files[i];
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
		entry_point = project_name + ".App";

	String project_file = project_name + (use_csharp ? ".csproj" : ".vcxproj");
	String project_type_guid = use_csharp ? project_type_guid_cs : project_type_guid_cpp;

	String manifest_path = AppendFileName(solution_dir, "Package.appxmanifest");
	Index<String> none_filters;
	for(int i = 0; i < none.GetCount(); i++)
		none_filters.FindAdd(none[i]);
	none_filters.FindAdd(manifest_path);

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

	VectorMap<String, String> sln_tokens;
	sln_tokens.Add("PROJECT_NAME", project_name);
	sln_tokens.Add("PROJECT_GUID", project_guid);
	sln_tokens.Add("PROJECT_TYPE_GUID", project_type_guid);
	sln_tokens.Add("PROJECT_FILE", project_file);

	String sln = ReplaceUwpTokens(UwpTemplate(uwp_sln_tpl, uwp_sln_tpl_length), sln_tokens);
	SaveFile(AppendFileName(solution_dir, project_name + ".sln"), sln);

	VectorMap<String, String> proj_tokens;
	proj_tokens.Add("PROJECT_GUID", XmlEscape(project_guid));
	proj_tokens.Add("ROOT_NAMESPACE", XmlEscape(project_name));
	proj_tokens.Add("PROJECT_NAME", XmlEscape(project_name));
	proj_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
	proj_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));

	if(use_csharp) {
		proj_tokens.Add("TARGET_FRAMEWORK", XmlEscape(uwp_csharp_framework));
		proj_tokens.Add("CSCOMPILE", String());
		proj_tokens.Add("NONEITEMS", MakeItemList(none, solution_dir, "None"));
		proj_tokens.Add("CONTENT_ITEMS", MakeContentItemList(assets, solution_dir));
		String csproj = ReplaceUwpTokens(UwpTemplate(uwp_csproj_tpl, uwp_csproj_tpl_length), proj_tokens);
		SaveFile(AppendFileName(solution_dir, project_name + ".csproj"), csproj);
	}
	else {
		proj_tokens.Add("PLATFORM_TOOLSET", XmlEscape(toolset));
		proj_tokens.Add("INCLUDE_DIRS", XmlEscape(include_dirs));
		proj_tokens.Add("DEFINES", XmlEscape(defines));
		proj_tokens.Add("CLCOMPILE", MakeItemList(clcompile, solution_dir, "ClCompile"));
		proj_tokens.Add("CLINCLUDE", MakeItemList(clincludes, solution_dir, "ClInclude"));
		proj_tokens.Add("NONEITEMS", MakeItemList(none, solution_dir, "None"));
		proj_tokens.Add("CONTENT_ITEMS", MakeContentItemList(assets, solution_dir));
		String vcxproj = ReplaceUwpTokens(UwpTemplate(uwp_vcxproj_tpl, uwp_vcxproj_tpl_length), proj_tokens);
		SaveFile(AppendFileName(solution_dir, project_name + ".vcxproj"), vcxproj);
	}

	VectorMap<String, String> manifest_tokens;
	manifest_tokens.Add("PACKAGE_NAME", XmlEscape(project_name));
	manifest_tokens.Add("PROJECT_NAME", XmlEscape(project_name));
	manifest_tokens.Add("TARGET_PLATFORM_VERSION", XmlEscape(target_version));
	manifest_tokens.Add("TARGET_PLATFORM_MIN_VERSION", XmlEscape(min_version));
	manifest_tokens.Add("ENTRY_POINT", XmlEscape(entry_point));

	String manifest = ReplaceUwpTokens(UwpTemplate(uwp_appxmanifest_tpl, uwp_appxmanifest_tpl_length), manifest_tokens);
	SaveFile(manifest_path, manifest);

	String filter_sources_guid = UwpGuidString("UWPFilter:Source Files:" + project_name);
	String filter_headers_guid = UwpGuidString("UWPFilter:Header Files:" + project_name);
	String filter_resources_guid = UwpGuidString("UWPFilter:Resource Files:" + project_name);
	String filter_content_guid = UwpGuidString("UWPFilter:Content Files:" + project_name);

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

	if(!use_csharp) {
		VectorMap<String, String> filter_tokens;
		filter_tokens.Add("FILTERS", String(filters));
		filter_tokens.Add("FILTER_CLCOMPILE", MakeFilteredItemList(clcompile, solution_dir, "ClCompile", "Source Files"));
		filter_tokens.Add("FILTER_CLINCLUDE", MakeFilteredItemList(clincludes, solution_dir, "ClInclude", "Header Files"));
		filter_tokens.Add("FILTER_NONE", MakeFilteredItemList(none_filters, solution_dir, "None", "Resource Files"));
		filter_tokens.Add("FILTER_CONTENT", MakeFilteredContentItemList(assets, solution_dir, "Content Files"));

		String vcxfilters = ReplaceUwpTokens(UwpTemplate(uwp_filters_tpl, uwp_filters_tpl_length), filter_tokens);
		SaveFile(AppendFileName(solution_dir, project_name + ".vcxproj.filters"), vcxfilters);
	}

#ifdef PLATFORM_WIN32
	String conf = HasFlag("DEBUG") ? "Debug" : "Release";
	String sln_path = AppendFileName(solution_dir, project_name + ".sln");
	String cmd;
	cmd << "msbuild " << GetPathQ(sln_path) << " /p:Configuration=" << conf << " /p:Platform=x64";
	PutConsole("UWP: building solution with msbuild...");
	if(Execute(cmd) != 0) {
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

