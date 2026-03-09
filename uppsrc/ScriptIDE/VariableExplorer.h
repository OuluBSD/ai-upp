#ifndef _ScriptIDE_VariableExplorer_h_
#define _ScriptIDE_VariableExplorer_h_

class VariableExplorer : public DockableCtrl {
public:
	typedef VariableExplorer CLASSNAME;
	VariableExplorer();

	void SetVariables(const VectorMap<PyValue, PyValue>& vars);
	void Clear();

private:
	ToolBar toolbar;
	ArrayCtrl list;
	Vector<PyValue> var_values; // Actual objects for inspection/editing
	
	void OnLeftDouble();
	void OnContextMenu(Bar& bar);
	void RemoveSelected();
	void InspectSelected(); // Opens detailed viewer dialog
	
	void LayoutToolbar(Bar& bar);
	void LayoutPaneMenu(Bar& bar);

	// Helper to get type string and icon
	String GetTypeString(const PyValue& v);
	Image  GetTypeIcon(const PyValue& v);
};

class DataViewerDialog : public TopWindow {
public:
	typedef DataViewerDialog CLASSNAME;
	DataViewerDialog(const String& name, const PyValue& val);

private:
	ArrayCtrl content;
	void PopulateList(const PyValue& val);
	void PopulateDict(const PyValue& val);
};

#endif
