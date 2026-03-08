#include "Maestro.h"

namespace Upp {

void TuCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI tu [-h]\n"
	       << "                     {build,info,query,complete,references,lsp,cache,transform,print-ast,draft}\\n"
	       << "                     ...\n"
	       << "positional arguments:\n"
	       << "  {build,info,query,complete,references,lsp,cache,transform,print-ast,draft}\\n"
	       << "                        TU subcommands\n"
	       << "    build               Build translation unit for package\n"
	       << "    info                Show translation unit information\n"
	       << "    query               Query symbols in translation unit\n"
	       << "    complete            Get auto-completion at location\n"
	       << "    references          Find all references to symbol\n"
	       << "    lsp                 Start Language Server Protocol server\n"
	       << "    cache               TU cache management\n"
	       << "    transform           Transform code to follow conventions (e.g., U++)\n"
	       << "    print-ast           Print AST for a source file\n"
	       << "    draft               Create draft classes and functions for a translation unit\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void TuCommand::Execute(const Vector<String>& args) {
	CommandLineArguments cla;
	cla.AddPositional("subcommand", UNKNOWN_V);
	cla.AddPositional("arg1", UNKNOWN_V);
	cla.Parse(args);
	
	if (cla.GetPositionalCount() == 0) { ShowHelp(); return; }
	
	String sub = AsString(cla.GetPositional(0));
	TuManager tum;
	
	if (sub == "build") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires package name.\n"; return; }
		tum.Build(AsString(cla.GetPositional(1)));
	}
	else if (sub == "info") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires package name.\n"; return; }
		tum.Info(AsString(cla.GetPositional(1)));
	}
	else if (sub == "query") {
		if(cla.GetPositionalCount() < 2) { Cerr() << "Error: Requires query string.\n"; return; }
		tum.Query(AsString(cla.GetPositional(1)));
	}
	else {
		Cout() << "Subcommand '" << sub << "' is not yet fully implemented in C++ but is on the roadmap.\n";
	}
}

}