#ifndef _ide_MCP_LayBridge_h_
#define _ide_MCP_LayBridge_h_

// Thread-safe facade for the Layout Designer (LayDes/LayDesigner).
// All methods post to the GUI thread via RunOnGui().
//
// The layout editor is only active when the currently open file is a .lay file.
// "li" = layout index (0-based), "ii" = item index within a layout (0-based).

class LayBridge {
public:
	// --- Layout file ---

	// List all .lay files in the current workspace's active package directory tree.
	ValueArray  ListLayFiles() const;

	// Open a .lay file in the IDE (causes LayDesigner to become the active designer).
	String      OpenLayFile(const String& path);

	// Returns path of the .lay file currently open in the designer, or "".
	String      GetOpenLayFile() const;

	// --- Layout list ---

	// Number of layouts in the open .lay file.
	int         GetLayoutCount() const;

	// Name of layout i.
	String      GetLayoutName(int i) const;

	// Size (width, height) of layout i.
	String      GetLayoutSize(int i) const;   // returns "WxH"

	// Index of the currently selected layout.
	int         GetCurrentLayout() const;

	// Select layout i as current.
	String      SetCurrentLayout(int i);

	// Add a new layout at end with given name.
	String      AddLayout(const String& name);

	// Insert a new layout before index i.
	String      InsertLayout(int before, const String& name);

	// Duplicate layout i with a new name.
	String      DuplicateLayout(int i, const String& newname);

	// Rename layout i.
	String      RenameLayout(int i, const String& newname);

	// Remove layout i.
	String      RemoveLayout(int i);

	// Set size of layout i.
	String      SetLayoutSize(int i, int w, int h);

	// --- Items in a layout ---

	// Number of items in layout li.
	int         GetItemCount(int li) const;

	// Returns item info: {index, type, variable, hide, rect:{l,t,r,b}}
	ValueMap    GetItem(int li, int ii) const;

	// Add an item of type_name with variable var at rect r to layout li.
	String      AddItem(int li, const String& type_name, const String& var,
	                    int left, int top, int right, int bottom);

	// Remove item ii from layout li.
	String      RemoveItem(int li, int ii);

	// Set rect of item ii in layout li.
	String      SetItemRect(int li, int ii, int left, int top, int right, int bottom);

	// Set variable name of item ii.
	String      SetItemVar(int li, int ii, const String& var);

	// --- Item properties ---

	// List all properties of item ii: [{name, value}]
	ValueArray  GetItemProperties(int li, int ii) const;

	// Set a property by name.
	String      SetItemProperty(int li, int ii, const String& name, const String& value);

	// --- Widget classes ---

	// List all available widget class names (from LayoutTypes()).
	ValueArray  ListClasses() const;

	// --- Persistence ---
	String      Save();

private:
	bool RunOnGui(Function<void()> fn, int timeout_ms = 8000) const;
};

extern LayBridge sLayBridge;

#endif
