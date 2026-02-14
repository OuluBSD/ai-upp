#ifndef _Maestro_CommandDispatcher_h_
#define _Maestro_CommandDispatcher_h_

#include "Maestro.h"

namespace Upp {

class CommandDispatcher {
	Array<Command> commands;

public:
	static CommandDispatcher& Get();

	void Register(Command* cmd) { commands.Add(cmd); }
	template <class T> void Register() { commands.Create<T>(); }

	void Execute(const String& cmdName, const Vector<String>& args);
	void MainHelp();
	
	const Array<Command>& GetCommands() const { return commands; }
	Command* FindCommand(const String& name);
};

void RegisterAllMaestroCommands(CommandDispatcher& d);

}

#endif
