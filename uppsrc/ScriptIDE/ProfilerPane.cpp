#include "ScriptIDE.h"

namespace Upp {

ProfilerPane::ProfilerPane()
{
	Title("Profiler");
	Icon(CtrlImg::plus()); // Temporary icon
	
	Add(list.SizePos());
	
	list.AddColumn("Function", 30).Sorting();
	list.AddColumn("Total time", 15).Sorting();
	list.AddColumn("Local time", 15).Sorting();
	list.AddColumn("Calls", 10).Sorting();
	list.AddColumn("File:Line", 30).Sorting();
	
	list.AllSorting();
	list.EvenRowColor();
	list.SetLineCy(20);
}

void ProfilerPane::SetData(const VectorMap<String, Value>& data)
{
	list.Clear();
	// TODO: Implement actual profiling data mapping
}

void ProfilerPane::Clear()
{
	list.Clear();
}

}
