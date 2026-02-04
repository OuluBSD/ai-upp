#include "Maestro.h"

namespace Upp {

TuManager::TuManager(const String& maestro_root)
{
	base_path = NormalizePath(maestro_root);
	cache_dir = AppendFileName(base_path, ".maestro/tu/cache");
	RealizeDirectory(cache_dir);
}

void TuManager::Build(const String& package)
{
	Cout() << "Building TU for package: " << package << "\n";
	
	// Stub implementation: Scan package directory
	String pkg_dir = AppendFileName(base_path, package); // Simplified path resolution
	if(!DirectoryExists(pkg_dir)) {
		// Try finding it via RepoScanner
		RepoScanner scanner;
		scanner.Scan(GetDocsRoot(FindPlanRoot()));
		bool found = false;
		for(const auto& p : scanner.packages) {
			if(p.name == package) {
				pkg_dir = p.dir;
				found = true;
				break;
			}
		}
		if(!found) {
			Cerr() << "Error: Package not found: " << package << "\n";
			return;
		}
	}
	
	Cout() << "Scanning " << pkg_dir << "...\n";
	
	int source_files = 0;
	int headers = 0;
	
	FindFile ff(AppendFileName(pkg_dir, "*.*"));
	while(ff) {
		String ext = GetFileExt(ff.GetName());
		if(ext == ".cpp" || ext == ".c" || ext == ".cc") source_files++;
		else if(ext == ".h" || ext == ".hpp" || ext == ".hh") headers++;
		ff.Next();
	}
	
	Cout() << "Found " << source_files << " source files and " << headers << " headers.\n";
	
	// Create a dummy cache entry
	String cache_file = AppendFileName(cache_dir, package + ".tu.json");
	ValueMap tu;
	tu.Add("package", package);
	tu.Add("source_files", source_files);
	tu.Add("headers", headers);
	tu.Add("timestamp", GetSysTime());
	StoreAsJsonFile(tu, cache_file, true);
	
	Cout() << "TU built and cached at " << cache_file << "\n";
}

void TuManager::Info(const String& package)
{
	String cache_file = AppendFileName(cache_dir, package + ".tu.json");
	if(!FileExists(cache_file)) {
		Cout() << "No TU cache found for " << package << ". Run 'tu build " << package << "' first.\n";
		return;
	}
	
	ValueMap tu;
	if(!LoadFromJsonFile(tu, cache_file)) {
		Cerr() << "Error: Failed to load TU cache.\n";
		return;
	}
	Cout() << "TU Info for " << package << ":\n";
	Cout() << "  Sources: " << tu["source_files"] << "\n";
	Cout() << "  Headers: " << tu["headers"] << "\n";
	Cout() << "  Built:   " << tu["timestamp"] << "\n";
}

void TuManager::Query(const String& query)
{
	Cout() << "Querying TUs for symbol: " << query << "\n";
	// Stub: Iterate all cached TUs
	FindFile ff(AppendFileName(cache_dir, "*.tu.json"));
	while(ff) {
		Cout() << "  Checking " << ff.GetName() << "...\n";
		ff.Next();
	}
	Cout() << "No matches found (stub).\n";
}

}
