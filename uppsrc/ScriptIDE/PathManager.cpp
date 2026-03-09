#include "ScriptIDE.h"

namespace Upp {

void PathManager::AddPath(const String& path)
{
	for(const String& p : paths)
		if(p == path) return;
	paths.Add(path);
}

void PathManager::RemovePath(int index)
{
	if(index >= 0 && index < paths.GetCount())
		paths.Remove(index);
}

void PathManager::SyncToVM(PyVM& vm)
{
	PyValue sys = vm.GetGlobals().Get(PyValue("sys"), PyValue::None());
	if(sys.GetType() == PY_DICT) {
		PyValue path_list = PyValue::List();
		for(const String& p : paths)
			path_list.Add(PyValue(p));
		sys.SetItem(PyValue("path"), path_list);
	}
}

void PathManager::Serialize(Stream& s)
{
	s % paths;
}

}
