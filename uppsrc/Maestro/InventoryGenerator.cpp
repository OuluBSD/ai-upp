#include "Maestro.h"

namespace Upp {

static String MaestroGetRelativePath(const String& path, const String& base)
{
	String p = NormalizePath(path);
	String b = NormalizePath(base);
	if(p.StartsWith(b)) {
		String rel = p.Mid(b.GetCount());
		if(rel.StartsWith("/") || rel.StartsWith("\\"))
			return rel.Mid(1);
		return rel;
	}
	return p;
}

String InventoryGenerator::GetFileHash(const String& filepath)
{
	return SHA256String(LoadFile(filepath));
}

String InventoryGenerator::DetectLanguage(const String& filename)
{
	static VectorMap<String, String> ext_map;
	ONCELOCK {
		ext_map.Add(".py", "Python");
		ext_map.Add(".js", "JavaScript");
		ext_map.Add(".ts", "TypeScript");
		ext_map.Add(".java", "Java");
		ext_map.Add(".cpp", "C++");
		ext_map.Add(".c", "C");
		ext_map.Add(".cs", "C#");
		ext_map.Add(".go", "Go");
		ext_map.Add(".rs", "Rust");
		ext_map.Add(".rb", "Ruby");
		ext_map.Add(".php", "PHP");
		ext_map.Add(".html", "HTML");
		ext_map.Add(".css", "CSS");
		ext_map.Add(".json", "JSON");
		ext_map.Add(".yaml", "YAML");
		ext_map.Add(".yml", "YAML");
		ext_map.Add(".toml", "TOML");
		ext_map.Add(".xml", "XML");
		ext_map.Add(".md", "Markdown");
		ext_map.Add(".txt", "Text");
		ext_map.Add(".sh", "Shell");
		ext_map.Add(".sql", "SQL");
		ext_map.Add(".bat", "Batch");
		ext_map.Add(".ps1", "PowerShell");
		ext_map.Add(".lua", "Lua");
		ext_map.Add(".h", "C++");
		ext_map.Add(".hpp", "C++");
	}

	String ext = ToLower(GetFileExt(filename));
	int q = ext_map.Find(ext);
	if(q >= 0) return ext_map[q];
	
	if(filename == "Dockerfile") return "Dockerfile";
	
	return "Unknown";
}

Vector<String> InventoryGenerator::ClassifyFileRole(const String& filepath, const String& language)
{
	Vector<String> roles;
	String path_lower = ToLower(filepath);
	String name_lower = ToLower(GetFileName(filepath));

	auto contains_any = [&](const String& s, const Vector<String>& words) {
		for(const auto& w : words) if(s.Find(w) >= 0) return true;
		return false;
	};

	if(contains_any(path_lower, {"build", "cmake", "make", "gradle", "pom", "cargo", "package", "setup", "requirements"}))
		roles.Add("build");

	if(contains_any(path_lower, {"config", "setting", "cfg", "conf", "env", "rc", "ini", "properties"}) && language != "Unknown")
		roles.Add("configuration");

	if(contains_any(name_lower, {"test", "spec", "testing", "_test", ".test", "unit", "integration"}))
		roles.Add("test");

	if(contains_any(name_lower, {"readme", "doc", "documentation", "guide", "tutorial", "manual", "license", "changelog"}))
		roles.Add("documentation");

	if(contains_any(name_lower, {"main", "index", "app", "start", "server", "init", "__main__", "entry", "bootstrap"}))
		roles.Add("entrypoint");

	if(roles.IsEmpty() && language != "Unknown" && !contains_any(language, {"Markdown", "Text", "Configuration", "JSON", "YAML"}))
		roles.Add("source");

	if(roles.IsEmpty()) {
		if(path_lower.Find("bin") >= 0 || path_lower.Find("exec") >= 0) roles.Add("executable");
		else if(path_lower.Find("asset") >= 0 || path_lower.Find("static") >= 0) roles.Add("asset");
		else if(path_lower.Find("lib") >= 0) roles.Add("library");
		else roles.Add("unknown");
	}

	return roles;
}

 MaestroInventory InventoryGenerator::Generate(const String& repo_path)
{
	MaestroInventory inv;
	inv.repository_path = repo_path;
	
	Vector<String> files;
	auto walk = [&](const String& dir) {
		Vector<String> stack;
		stack.Add(dir);
		while(stack.GetCount()) {
			String current = stack.Pop();
			FindFile ff(AppendFileName(current, "*"));
			while(ff) {
				String name = ff.GetName();
				if(name != "." && name != ".." && !name.StartsWith(".") && 
				   name != "node_modules" && name != "build" && name != "out" && name != "bin" && name != "obj") {
					String path = AppendFileName(current, name);
					if(ff.IsDirectory()) stack.Add(path);
					else if(ff.IsFile()) {
						MaestroInventoryFileInfo& info = inv.files.Add();
						info.full_path = NormalizePath(path);
						info.path = MaestroGetRelativePath(info.full_path, NormalizePath(repo_path));
						info.size = ff.GetLength();
						info.hash = GetFileHash(info.full_path);
						info.language = DetectLanguage(name);
						info.roles = ClassifyFileRole(info.path, info.language);
						
						inv.total_count++;
						String ext = ToLower(GetFileExt(name));
						inv.by_extension.GetAdd(ext, 0)++;
						inv.by_language.GetAdd(info.language, 0)++;
						for(const auto& r : info.roles) inv.by_role.GetAdd(r, 0)++;
						String directory = ToLower(GetFileDirectory(info.path));
						if(directory.IsEmpty()) directory = "/";
						inv.by_directory.GetAdd(directory, 0)++;
					}
				}
				ff.Next();
			}
		}
	};
	
	walk(repo_path);
	
	int64 total = 0;
	for(const auto& f : inv.files) total += f.size;
	inv.size_summary("total_bytes") = total;
	
	inv.summary("total_files") = inv.total_count;
	inv.summary("total_size_bytes") = total;
	
	return inv;
}

}
