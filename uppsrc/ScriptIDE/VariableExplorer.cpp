#include "ScriptIDE.h"

namespace Upp {

VariableExplorer::VariableExplorer()
{
	Add(list.SizePos());
	
	list.AddIndex("INTERNAL_IDX");
	list.AddColumn("Name", 30).Sorting();
	list.AddColumn("Type", 15).Sorting();
	list.AddColumn("Size", 10).Sorting();
	list.AddColumn("Value", 45).Sorting();
	
	list.AllSorting();
	list.EvenRowColor();
	list.SetLineCy(20);
	
	list.WhenLeftDouble = [=] { OnLeftDouble(); };
	list.WhenBar = [=](Bar& bar) { OnContextMenu(bar); };
}

void VariableExplorer::SetVariables(const VectorMap<PyValue, PyValue>& vars)
{
	list.Clear();
	var_values.Clear();
	
	for(int i = 0; i < vars.GetCount(); i++) {
		const PyValue& key = vars.GetKey(i);
		const PyValue& val = vars[i];
		
		String name = key.ToString();
		String type = GetTypeString(val);
		String size = "";
		
		int type_kind = val.GetType();
		if(type_kind == PY_LIST || type_kind == PY_TUPLE || type_kind == PY_DICT || type_kind == PY_SET || type_kind == PY_STR)
			size = AsString(val.GetCount());
		
		String repr = val.Repr();
		if(repr.GetCount() > 200)
			repr = repr.Mid(0, 197) + "...";
			
		int internal_idx = var_values.GetCount();
		var_values.Add(val);
		
		list.Add(internal_idx, name, type, size, repr);
	}
}

void VariableExplorer::Clear()
{
	list.Clear();
	var_values.Clear();
}

String VariableExplorer::GetTypeString(const PyValue& v)
{
	return PyTypeName(v.GetType());
}

Image VariableExplorer::GetTypeIcon(const PyValue& v)
{
	// TODO: Add custom icons
	switch(v.GetType()) {
		case PY_INT:
		case PY_FLOAT:
		case PY_COMPLEX: return CtrlImg::plus();
		case PY_STR:     return CtrlImg::File();
		case PY_LIST:
		case PY_TUPLE:   return CtrlImg::Dir();
		case PY_DICT:    return CtrlImg::Dir();
		case PY_FUNCTION: return CtrlImg::right_arrow();
		default:         return Image();
	}
}

void VariableExplorer::OnLeftDouble()
{
	InspectSelected();
}

void VariableExplorer::OnContextMenu(Bar& bar)
{
	if(list.IsCursor()) {
		bar.Add("Inspect", [=] { InspectSelected(); });
		bar.Add("Remove", [=] { RemoveSelected(); });
		bar.Separator();
	}
	bar.Add("Clear All", [=] { Clear(); });
}

void VariableExplorer::RemoveSelected()
{
	if(list.IsCursor())
		list.Remove(list.GetCursor());
}

void VariableExplorer::InspectSelected()
{
	if(!list.IsCursor()) return;
	
	int internal_idx = list.Get(list.GetCursor(), "INTERNAL_IDX");
	if(internal_idx < 0 || internal_idx >= var_values.GetCount()) return;
	
	const PyValue& val = var_values[internal_idx];
	String name = list.Get(list.GetCursor(), "Name").ToString();
	
	int type = val.GetType();
	if(type == PY_LIST || type == PY_TUPLE || type == PY_DICT) {
		DataViewerDialog* dlg = new DataViewerDialog(name, val);
		dlg->Open(); // Open as modeless dockable dialog if possible, or just modal
	} else {
		PromptOK("Variable: " + name + "\nType: " + PyTypeName(type) + "\n\nValue:\n" + val.Repr());
	}
}

// --- DataViewerDialog Implementation ---

DataViewerDialog::DataViewerDialog(const String& name, const PyValue& val)
{
	Title("Data Viewer: " + name);
	Sizeable().Zoomable().CenterScreen();
	SetRect(0, 0, 600, 400);
	
	Add(content.SizePos());
	content.AllSorting();
	content.EvenRowColor();
	content.SetLineCy(20);
	
	if(val.GetType() == PY_LIST || val.GetType() == PY_TUPLE)
		PopulateList(val);
	else if(val.GetType() == PY_DICT)
		PopulateDict(val);
}

void DataViewerDialog::PopulateList(const PyValue& val)
{
	content.AddColumn("Index", 20).Sorting();
	content.AddColumn("Value", 80).Sorting();
	
	for(int i = 0; i < val.GetCount(); i++) {
		content.Add(i, val.GetItem(i).Repr());
	}
}

void DataViewerDialog::PopulateDict(const PyValue& val)
{
	content.AddColumn("Key", 40).Sorting();
	content.AddColumn("Value", 60).Sorting();
	
	const VectorMap<PyValue, PyValue>& d = val.GetDict();
	for(int i = 0; i < d.GetCount(); i++) {
		content.Add(d.GetKey(i).Repr(), d[i].Repr());
	}
}

}
