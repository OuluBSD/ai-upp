#include "Maestro.h"
#include "CommandDispatcher.h"
#include <ByteVM/ByteVM.h>

namespace Upp {

static PyValue builtin_maestro_call(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 1) return PyValue::None();
	
	String cmdName = args[0].ToString();
	Vector<String> sub_args;
	if(args.GetCount() >= 2 && args[1].GetType() == PY_LIST) {
		const Vector<PyValue>& va = args[1].GetArray();
		for(int i = 0; i < va.GetCount(); i++)
			sub_args.Add(va[i].ToString());
	}
	
	CommandDispatcher& d = CommandDispatcher::Get();
	if(d.GetCommands().IsEmpty()) {
		RegisterAllMaestroCommands(d);
	}
	
	d.Execute(cmdName, sub_args);
	return PyValue::None();
}

void RegisterMaestroModule(PyVM& vm) {
	PyValue maestro = vm.GetGlobals().Get(PyValue("maestro"), PyValue::Dict());
	maestro.SetItem(PyValue("call"), PyValue::Function("call", builtin_maestro_call));
	vm.GetGlobals().GetAdd(PyValue("maestro")) = maestro;
}

}
