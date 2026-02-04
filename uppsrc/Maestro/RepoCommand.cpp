#include "Maestro.h"

namespace Upp {

void RepoCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI repo [-h] {list,ls,show,sh,pkg,asm,resolve,refresh,hier,conventions,rules,profile,evidence}\n"
	       << "\n"
	       << "Repository subcommands:\n"
	       << "    list (ls)           List all packages in repository\n"
	       << "    show (sh)           Show repository scan results\n"
	       << "    pkg <name>          Show detailed package info\n"
	       << "    asm                 Assembly management\n"
	       << "    resolve             Scan repository for packages across build systems\n"
	       << "    refresh             Refresh repository metadata\n"
	       << "    hier                Repository hierarchy\n"
	       << "    conventions         Naming conventions\n"
	       << "    rules               Repository rules\n"
	       << "    profile             Repo profile management\n"
	       << "    evidence            Evidence pack generation\n";
}

void RepoCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) {
		ShowHelp();
		return;
	}
	
	String plan_root = FindPlanRoot();
	if(plan_root.IsEmpty()) {
		Cerr() << "Error: Could not find project root.\n";
		return;
	}
	String docs_root = GetDocsRoot(plan_root);

	RepoScanner scanner;
	scanner.Scan(docs_root);

	String sub = AsString(cla.GetPositional(0));
	if (sub == "list" || sub == "ls") {
		Cout() << "Packages found in " << docs_root << ":\n";
		for(const auto& pkg : scanner.packages) {
			Cout() << Format(" [%-10s] %s (%s)\n", pkg.build_system, pkg.name, pkg.dir);
		}
	}
	else if (sub == "show" || sub == "sh") {
		Cout() << "Repository Scan Summary:\n"
		       << "  Project Root: " << docs_root << "\n"
		       << "  Total Packages: " << scanner.packages.GetCount() << "\n"
		       << "  Total Assemblies: " << scanner.assemblies.GetCount() << "\n";
		
		if(scanner.assemblies.GetCount() > 0) {
			Cout() << "\nAssemblies:\n";
			for(const auto& asm_info : scanner.assemblies) {
				Cout() << "  - " << asm_info.name << " (" << asm_info.packages.GetCount() << " packages)\n";
			}
		}
	}
	else if (sub == "pkg") {
		if(cla.GetPositionalCount() < 2) {
			Cerr() << "Error: 'repo pkg' requires a package name.\n";
			return;
		}
		String name = AsString(cla.GetPositional(1));
		const PackageInfo* found = nullptr;
		for(const auto& pkg : scanner.packages) {
			if(pkg.name == name) {
				found = &pkg;
				break;
			}
		}
		
		if(found) {
			Cout() << "Package:      " << found->name << "\n"
			       << "Directory:    " << found->dir << "\n"
			       << "Build System: " << found->build_system << "\n"
			       << "Dependencies: " << Join(found->dependencies, ", ") << "\n"
			       << "File Groups:  " << found->groups.GetCount() << "\n";
			for(const auto& group : found->groups) {
				Cout() << "  - Group: " << group.name << " (" << group.files.GetCount() << " files)\n";
			}
		} else {
			Cerr() << "Error: Package '" << name << "' not found.\n";
		}
	}
	else if (sub == "deps" || sub == "dependents") {
		if(cla.GetPositionalCount() < 2) {
			Cerr() << "Error: '" << sub << "' requires a package name.\n";
			return;
		}
		String name = AsString(cla.GetPositional(1));
		Vector<String> list;
		if(sub == "deps") list = DependencyResolver::GetDependencies(scanner.packages, name);
		else list = DependencyResolver::GetDependents(scanner.packages, name);
		
		Cout() << "Transitive " << sub << " for " << name << ":\n";
		for(const auto& item : list) Cout() << "  - " << item << "\n";
	}
	else if (sub == "sort") {
		try {
			Array<PackageInfo> sorted = DependencyResolver::TopologicalSort(scanner.packages);
			Cout() << "Build order (" << sorted.GetCount() << " packages):\n";
			for(int i = 0; i < sorted.GetCount(); i++) Cout() << Format("%4d. %s\n", i + 1, sorted[i].name);
		} catch (const Exc& e) {
			Cerr() << "Error: " << e << "\n";
		}
	}
	else if (sub == "asm") {
		Array<UppAssemblyReader::AssemblyInfo> asms = UppAssemblyReader::ReadAll();
		Cout() << "U++ IDE Assemblies (from ~/.config/u++/ide/*.var):\n";
		for(const auto& a : asms) {
			Cout() << "  - " << a.assembly_name << " (" << a.var_file << ")\n";
			for(const auto& p : a.upp_paths) {
				Cout() << "    " << p << "\n";
			}
		}
	}
	else if (sub == "resolve" || sub == "refresh") {
		Cout() << "Refreshing repository metadata...\n";
		scanner.Scan(docs_root);
		Cout() << "✓ Repository resolved: " << scanner.packages.GetCount() << " packages found.\n";
	}
	else if (sub == "hier" || sub == "conventions" || sub == "rules" || sub == "profile" || sub == "evidence") {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
	else {
		Cout() << "Unknown repo subcommand: " << sub << "\n";
		ShowHelp();
	}
}

} // namespace Upp
