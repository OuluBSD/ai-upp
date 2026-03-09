#ifdef flagGUI
#include "MCP.h"
#include <ide/ide.h>

NAMESPACE_UPP

LayBridge sLayBridge;

// ---- helpers ---------------------------------------------------------------

bool LayBridge::RunOnGui(Function<void()> fn, int timeout_ms) const
{
	Semaphore done;
	PostCallback([fn = pick(fn), &done]() mutable {
		fn();
		done.Release();
	});
	return done.Wait(timeout_ms);
}

static LayDes* GetLayDes()
{
	Ide* ide = TheIde();
	if(!ide) return nullptr;
	LayDesigner* ld = dynamic_cast<LayDesigner*>(ide->designer.Get());
	if(!ld) return nullptr;
	return &ld->GetLayDes();
}

// ---- layout file -----------------------------------------------------------

ValueArray LayBridge::ListLayFiles() const
{
	ValueArray result;
	// Collect .lay files from all packages in the current workspace
	RunOnGui([&] {
		const Workspace& wspc = GetIdeWorkspace();
		Index<String> seen;
		for(int pi = 0; pi < wspc.GetCount(); pi++) {
			String pkg_name = wspc[pi];
			const Package& pkg = wspc.GetPackage(pi);
			for(int fi = 0; fi < pkg.file.GetCount(); fi++) {
				String fn = SourcePath(pkg_name, pkg.file[fi]);
				if(ToLower(GetFileExt(fn)) == ".lay" && seen.Find(fn) < 0) {
					seen.Add(fn);
					result.Add(fn);
				}
			}
		}
	});
	return result;
}

String LayBridge::OpenLayFile(const String& path)
{
	String err;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) { err = "IDE not available"; return; }
		ide->EditFile(path);
	});
	return err;
}

String LayBridge::GetOpenLayFile() const
{
	String result;
	RunOnGui([&] {
		Ide* ide = TheIde();
		if(!ide) return;
		LayDesigner* ld = dynamic_cast<LayDesigner*>(ide->designer.Get());
		if(ld) result = ld->GetFileName();
	});
	return result;
}

// ---- layout list -----------------------------------------------------------

int LayBridge::GetLayoutCount() const
{
	int result = 0;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(ld) result = ld->McpGetLayoutCount();
	});
	return result;
}

String LayBridge::GetLayoutName(int i) const
{
	String result;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(ld && i >= 0 && i < ld->McpGetLayoutCount()) result = ld->McpGetLayoutName(i);
	});
	return result;
}

String LayBridge::GetLayoutSize(int i) const
{
	String result;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(ld && i >= 0 && i < ld->McpGetLayoutCount()) {
			Size sz = ld->McpGetLayoutSize(i);
			result = AsString(sz.cx) + "x" + AsString(sz.cy);
		}
	});
	return result;
}

int LayBridge::GetCurrentLayout() const
{
	int result = -1;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(ld) result = ld->McpGetCurrentLayout();
	});
	return result;
}

String LayBridge::SetCurrentLayout(int i)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(i < 0 || i >= ld->McpGetLayoutCount()) { err = "Layout index out of range"; return; }
		ld->McpSetCurrentLayout(i);
	});
	return err;
}

String LayBridge::AddLayout(const String& name)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		ld->McpAddLayout(name);
	});
	return err;
}

String LayBridge::InsertLayout(int before, const String& name)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		ld->McpInsertLayout(before, name);
	});
	return err;
}

String LayBridge::DuplicateLayout(int i, const String& newname)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(i < 0 || i >= ld->McpGetLayoutCount()) { err = "Layout index out of range"; return; }
		ld->McpDuplicateLayout(i, newname);
	});
	return err;
}

String LayBridge::RenameLayout(int i, const String& newname)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpRenameLayout(i, newname)) err = "Layout index out of range";
	});
	return err;
}

String LayBridge::RemoveLayout(int i)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpRemoveLayout(i)) err = "Layout index out of range";
	});
	return err;
}

String LayBridge::SetLayoutSize(int i, int w, int h)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(i < 0 || i >= ld->McpGetLayoutCount()) { err = "Layout index out of range"; return; }
		ld->McpSetLayoutSize(i, Size(w, h));
	});
	return err;
}

// ---- items -----------------------------------------------------------------

int LayBridge::GetItemCount(int li) const
{
	int result = 0;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(ld) result = ld->McpGetItemCount(li);
	});
	return result;
}

ValueMap LayBridge::GetItem(int li, int ii) const
{
	ValueMap result;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) return;
		if(li < 0 || li >= ld->McpGetLayoutCount()) return;
		if(ii < 0 || ii >= ld->McpGetItemCount(li)) return;
		Rect r = ld->McpGetItemRect(li, ii);
		ValueMap rect; rect.Add("l", r.left); rect.Add("t", r.top); rect.Add("r", r.right); rect.Add("b", r.bottom);
		result.Add("index",    ii);
		result.Add("type",     ld->McpGetItemType(li, ii));
		result.Add("variable", ld->McpGetItemVar(li, ii));
		result.Add("hide",     ld->McpGetItemHide(li, ii));
		result.Add("rect",     rect);
	});
	return result;
}

String LayBridge::AddItem(int li, const String& type_name, const String& var,
                          int left, int top, int right, int bottom)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(li < 0 || li >= ld->McpGetLayoutCount()) { err = "Layout index out of range"; return; }
		ld->McpAddItem(li, type_name, var, Rect(left, top, right, bottom));
	});
	return err;
}

String LayBridge::RemoveItem(int li, int ii)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpRemoveItem(li, ii)) err = "Item or layout index out of range";
	});
	return err;
}

String LayBridge::SetItemRect(int li, int ii, int left, int top, int right, int bottom)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpSetItemRect(li, ii, Rect(left, top, right, bottom)))
			err = "Item or layout index out of range";
	});
	return err;
}

String LayBridge::SetItemVar(int li, int ii, const String& var)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpSetItemVar(li, ii, var)) err = "Item or layout index out of range";
	});
	return err;
}

// ---- item properties -------------------------------------------------------

ValueArray LayBridge::GetItemProperties(int li, int ii) const
{
	ValueArray result;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) return;
		if(li < 0 || li >= ld->McpGetLayoutCount()) return;
		if(ii < 0 || ii >= ld->McpGetItemCount(li)) return;
		int n = ld->McpGetItemPropCount(li, ii);
		for(int pi = 0; pi < n; pi++) {
			ValueMap v;
			v.Add("name",  ld->McpGetItemPropName(li, ii, pi));
			v.Add("value", ld->McpGetItemPropValue(li, ii, pi));
			result.Add(v);
		}
	});
	return result;
}

String LayBridge::SetItemProperty(int li, int ii, const String& name, const String& value)
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		if(!ld->McpSetItemProp(li, ii, name, value)) err = "Property not found";
	});
	return err;
}

// ---- widget classes --------------------------------------------------------

ValueArray LayBridge::ListClasses() const
{
	ValueArray result;
	RunOnGui([&] {
		const VectorMap<String, LayoutType>& types = LayoutTypes();
		for(int i = 0; i < types.GetCount(); i++) {
			const LayoutType& t = types[i];
			ValueMap v;
			v.Add("name",  types.GetKey(i));
			v.Add("group", t.group);
			const char* kind_str = t.kind == LAYOUT_CTRL ? "ctrl" :
			                       t.kind == LAYOUT_SUBCTRL ? "subctrl" : "template";
			v.Add("kind",  kind_str);
			result.Add(v);
		}
	});
	return result;
}

// ---- persistence -----------------------------------------------------------

String LayBridge::Save()
{
	String err;
	RunOnGui([&] {
		LayDes* ld = GetLayDes();
		if(!ld) { err = "No layout file open"; return; }
		ld->McpSave();
	});
	return err;
}

END_UPP_NAMESPACE
#endif // flagGUI
