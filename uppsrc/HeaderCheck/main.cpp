#include "HeaderCheck.h"

#ifdef flagMAIN

CONSOLE_APP_MAIN
{
	using namespace UPP;
	
	// Parse command line arguments
	const auto& cmds = CommandLine();
	
	if (cmds.GetCount() < 2) {
		Cout() << "HeaderCheck - U++ Header Dependency Analyzer" << EOL;
		Cout() << "Usage: HeaderCheck <assembly_dirs> <package_name>" << EOL;
		Cout() << "  assembly_dirs: Comma-separated list of assembly directories" << EOL;
		Cout() << "  package_name: Name of the package to analyze" << EOL;
		Cout() << EOL;
		Cout() << "Example: HeaderCheck /path/to/upp/src,/path/to/my/assembly MyPackage" << EOL;
		return;
	}
	
	// Parse assembly directories (first argument, comma-separated)
	String assembly_dirs_str = cmds[0];
	Vector<String> assembly_dirs = Split(assembly_dirs_str, ',');
	
	// Remove any potential whitespace from directory paths and expand ~ if present
	for (auto& dir : assembly_dirs) {
		dir = TrimLeft(TrimRight(dir));
		if (dir.StartsWith("~/")) {
			dir = GetHomeDirectory() + dir.Mid(1);  // Replace ~/ with actual home directory
		}
	}
	
	// Parse package name (second argument)
	String package_name = cmds[1];
	
	Cout() << "Analyzing header dependencies for package: " << package_name << EOL;
	Cout() << "Using assembly directories:" << EOL;
	for (const auto& dir : assembly_dirs) {
		Cout() << "  - " << dir << EOL;
	}
	Cout() << EOL;
	
	// Perform the analysis
	AnalyzeHeaderDependencies(assembly_dirs, package_name);
}

#endif