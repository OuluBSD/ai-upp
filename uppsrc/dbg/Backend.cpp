#include "dbg.h"

using namespace Upp;

VectorMap<String, String> GetPlannedDbgBackends()
{
	VectorMap<String, String> backends;
	backends.Add("vs", "Visual Studio backend (planned)");
	backends.Add("gdb", "GDB backend (planned)");
	backends.Add("lldb", "LLDB backend (planned)");
	return backends;
}

String GetPlannedDbgBackendList()
{
	const VectorMap<String, String> backends = GetPlannedDbgBackends();
	String out;
	for(int i = 0; i < backends.GetCount(); i++) {
		if(i)
			out << ", ";
		out << backends.GetKey(i);
	}
	return out;
}
