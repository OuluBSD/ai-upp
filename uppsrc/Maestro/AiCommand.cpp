#include "Maestro.h"

namespace Upp {

void AiCommand::ShowHelp() const {
	Cout() << "usage: MaestroCLI ai [-h] {sync,qwen,gemini,codex,claude,qwen-old,help,h} ...\n"
	       << "positional arguments:\n"
	       << "  {sync,qwen,gemini,codex,claude,qwen-old,help,h}\n"
	       << "                        AI subcommands\n"
	       << "    sync                Sync to the next task in the active AI session\n"
	       << "    qwen                Run Qwen engine interactively\n"
	       << "    gemini              Run Gemini engine interactively\n"
	       << "    codex               Run Codex engine interactively\n"
	       << "    claude              Run Claude engine interactively\n"
	       << "    qwen-old            Run Qwen server or TUI client (legacy)\n"
	       << "    help (h)            Show help for AI commands\n"
	       << "options:\n"
	       << "  -h, --help            show this help message and exit\n";
}

void AiCommand::Execute(const Vector<String>& args) {
	Cout() << "Command 'ai' is not yet fully implemented in C++.\n";
}

}