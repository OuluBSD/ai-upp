#include "Maestro.h"

namespace Upp {

void TuCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI tu [-h]\n"
	       << "                     {build,info,query,complete,references,lsp,cache,transform,print-ast,draft}\n"
	       << "                     ...\n"
	       << "positional arguments:\n"
	       << "  {build,info,query,complete,references,lsp,cache,transform,print-ast,draft}\n"
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
	Cout() << "Command 'tu' is not yet fully implemented in C++.\n";
}

} 
