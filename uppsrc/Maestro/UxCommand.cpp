#include "Maestro.h"

namespace Upp {

void UxCommand::ShowHelp() const {
	Cout() << "Usage: MaestroCLI ux access [path] [value]\n"
	       << "  Without arguments, dumps all accessible GUI elements.\n"
	       << "  With path, attempts to find the element.\n"
	       << "  With path and value, attempts to write the value to the element.\n";
}

void UxCommand::Execute(const Vector<String>& args) {
	if(args.GetCount() > 0 && args[0] == "access") {
		MaestroToolRegistry tool_reg;
		RegisterUxTools(tool_reg);
		const MaestroTool* tool = tool_reg.Find("ux_access");
		if(!tool) {
			Cout() << "Error: ux_access tool not found.\n";
			return;
		}

		ValueMap req;
		if(args.GetCount() == 1) {
			req("action", "read");
		}
		else if(args.GetCount() == 2) {
			req("action", "read")("path", args[1]);
		}
		else if(args.GetCount() >= 3) {
			req("action", "write")("path", args[1])("value", args[2]);
		}

		Cout() << "Waiting for response...\n";
		Value res = tool->Execute(req);
		
		if(res.IsError()) {
			Cout() << "Error: " << ::Upp::GetErrorText(res) << "\n";
		}
		else if(res.Is<ValueArray>()) {
			ValueArray va = res;
			for(int i = 0; i < va.GetCount(); i++) {
				ValueMap m = va[i];
				Cout() << "  " << m["path"];
				if(m["is_menu"]) Cout() << " (menu)";
				else {
					Cout() << " [" << (m["enabled"] ? "enabled" : "disabled") << "]";
					if(!IsNull(m["value"])) Cout() << " value=" << m["value"];
					if(m["checked"]) Cout() << " (checked)";
				}
				Cout() << "\n";
			}
		}
		else {
			Cout() << "Result: " << res << "\n";
		}
		return;
	}
	ShowHelp();
}

#ifdef flagGUI
struct UxAccessTool : MaestroTool {
	String GetName() const override { return "ux_access"; }
	String GetDescription() const override { return "Inspect and interact with UI elements"; }
	Value  GetSchema() const override {
		ValueMap m;
		m("action", "string (read, write)")
		 ("path", "string (element path)")
		 ("value", "string (value to write)");
		return m;
	}
	Value  Execute(const ValueMap& params) const override {
		String action = params["action"];
		String path = params["path"];
		Value val = params["value"];
		
		GuiAutomationVisitor vis;
		Vector<Ctrl *> top = Ctrl::GetTopCtrls();
		
		if(action == "read") {
			if(path.IsEmpty()) {
				ValueArray va;
				for(Ctrl *c : top) {
					if(c->IsVisible()) {
						vis.Read(*c);
						for(const auto& el : vis.elements) {
							ValueMap m;
							m("path", el.path)("value", el.value)("checked", (int)el.checked)("enabled", (int)el.enabled)("is_menu", (int)el.is_menu);
							va.Add(m);
						}
					}
				}
				return va;
			}
			for(Ctrl *c : top) {
				if(c->IsVisible()) {
					Value v = vis.Read(*c, path);
					if(!IsNull(v)) return v;
				}
			}
			return ErrorValue("Element not found: " + path);
		}
		else if(action == "write") {
			for(Ctrl *c : top) {
				if(c->IsVisible()) {
					if(vis.Write(*c, path, val)) return true;
				}
			}
			return ErrorValue("Failed to write to element: " + path);
		}
		return ErrorValue("Unknown action: " + action);
	}
};

void RegisterUxTools(MaestroToolRegistry& reg) {
	reg.Add(new UxAccessTool());
}
#else
struct RemoteUxAccessTool : MaestroTool {
	String GetName() const override { return "ux_access"; }
	String GetDescription() const override { return "Remote: Inspect and interact with UI elements in MaestroHub"; }
	Value  GetSchema() const override {
		ValueMap m;
		m("action", "string (read, write)")
		 ("path", "string (element path)")
		 ("value", "string (value to write)");
		return m;
	}
	Value  Execute(const ValueMap& params) const override {
		String root = FindPlanRoot();
		if(root.IsEmpty()) return ErrorValue("Not in a Maestro project");

		String req_path = AppendFileName(root, "docs/maestro/ux_request.json");
		String res_path = AppendFileName(root, "docs/maestro/ux_response.json");
		
		DeleteFile(res_path);
		SaveFile(req_path, StoreAsJson(params));
		
		Time stop = GetSysTime() + 10;
		while(GetSysTime() < stop) {
			if(FileExists(res_path)) {
				Value res = ParseJSON(LoadFile(res_path));
				DeleteFile(res_path);
				return res;
			}
			Sleep(100);
		}
		return ErrorValue("Timeout: MaestroHub did not respond");
	}
};

void RegisterUxTools(MaestroToolRegistry& reg) {
	reg.Add(new RemoteUxAccessTool());
}
#endif

}
