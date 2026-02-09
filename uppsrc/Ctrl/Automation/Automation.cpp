#include "Automation.h"
#include <ByteVM/PyBindings.h>

NAMESPACE_UPP

PY_CLASS(AutomationElement, "AutomationElement")
PY_DATA_BEGIN(AutomationElement)
    if(name == "label") return PyValue(val.text);
    if(name == "path") return PyValue(val.path);
    if(name == "value") return PyValue::FromValue(val.value);
    if(name == "checked") return PyValue(val.checked);
    if(name == "enabled") return PyValue(val.enabled);
    if(name == "visible") return PyValue(val.visible);
PY_DATA_END

static PyValue AutomationElement_Ctor(const Vector<PyValue>& args, void* user_data) {
    return PyValue(new PyAutomationElement());
}

static PyValue AutomationElement_click(const Vector<PyValue>& args, void*) {
    PY_SELF(AutomationElement);
    GuiAutomationVisitor vis;
    vis.include_hidden = self.visible == false; // If we specifically want to click a hidden element, allow walking to it
    Vector<Ctrl *> top = Ctrl::GetTopCtrls();
    for(Ctrl *c : top) {
        if(c->IsVisible() || vis.include_hidden) {
            if(vis.Write(*c, self.path, ::Upp::Value(), true))
                return PyValue(true);
        }
    }
    return PyValue(false);
}

static PyValue AutomationElement_set(const Vector<PyValue>& args, void*) {
    PY_SELF(AutomationElement);
    if(args.GetCount() < 2) return PyValue(false);
    ::Upp::Value val = args[1].ToValue();
    GuiAutomationVisitor vis;
    vis.include_hidden = self.visible == false;
    Vector<Ctrl *> top = Ctrl::GetTopCtrls();
    for(Ctrl *c : top) {
        if(c->IsVisible() || vis.include_hidden) {
            if(vis.Write(*c, self.path, val, false))
                return PyValue(true);
        }
    }
    return PyValue(false);
}

static PyValue builtin_find(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 1) return PyValue::None();
    String path = args[0].ToString();
    bool include_hidden = args.GetCount() >= 2 ? args[1].IsTrue() : false;
    
    GuiAutomationVisitor vis;
    vis.include_hidden = include_hidden;
    Vector<Ctrl*> top = Ctrl::GetTopCtrls();
    for(Ctrl *c : top) {
        if(c->IsVisible() || include_hidden) {
            vis.Read(*c);
            for(const auto& el : vis.elements) {
                if(el.path == path || el.text == path) {
                    return PyValue(new PyAutomationElement(el));
                }
            }
        }
    }
    return PyValue::None();
}

static PyValue builtin_dump_ui(const Vector<PyValue>& args, void*) {
    bool include_hidden = args.GetCount() >= 1 ? args[0].IsTrue() : false;
    GuiAutomationVisitor vis;
    vis.include_hidden = include_hidden;
    Vector<Ctrl*> top = Ctrl::GetTopCtrls();
    String res;
    for(Ctrl *c : top) {
        if(c->IsVisible() || include_hidden) {
            vis.Read(*c);
            for(const auto& el : vis.elements) {
                res << el.path << " = " << el.value << (el.is_menu ? " (menu)" : "") << (el.visible ? "" : " (hidden)") << "\n";
            }
        }
    }
    return PyValue(res);
}

static PyValue builtin_wait_ready(const Vector<PyValue>& args, void*) {
    Ctrl::ProcessEvents();
    return PyValue::None();
}

static PyValue builtin_wait_time(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 1) return PyValue::None();
    int ms = (int)(args[0].AsDouble() * 1000.0);
    int64 stop = msecs() + ms;
    while(msecs() < stop) {
        Ctrl::ProcessEvents();
        Sleep(10);
    }
    return PyValue::None();
}

static AddMockFn sAddMock = nullptr;
void SetAddMockFn(AddMockFn fn) { sAddMock = fn; }

static PyValue builtin_mock_ai(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 2) return PyValue::None();
    String regex = args[0].ToString();
    String response = args[1].ToString();
    if(sAddMock) sAddMock(regex, response);
    return PyValue::None();
}

static PyValue builtin_find_all(const Vector<PyValue>& args, void*) {
    String parent_path = "";
    bool include_hidden = false;
    if(args.GetCount() >= 1) parent_path = args[0].ToString();
    if(args.GetCount() >= 2) include_hidden = args[1].IsTrue();
    
    GuiAutomationVisitor vis;
    vis.include_hidden = include_hidden;
    Vector<Ctrl*> top = Ctrl::GetTopCtrls();
    PyValue list = PyValue::List();
    for(Ctrl *c : top) {
        if(c->IsVisible() || include_hidden) {
            vis.Read(*c);
            for(const auto& el : vis.elements) {
                if(parent_path.IsEmpty() || el.path.StartsWith(parent_path + "/")) {
                    list.Add(PyValue(new PyAutomationElement(el)));
                }
            }
        }
    }
    return list;
}

void RegisterAutomationBindings(PyVM& vm) {
    auto& globals = vm.GetGlobals();
    
    globals.GetAdd(PyValue("find")) = PyValue::Function("find", builtin_find);
    globals.GetAdd(PyValue("find_all")) = PyValue::Function("find_all", builtin_find_all);
    globals.GetAdd(PyValue("dump_ui")) = PyValue::Function("dump_ui", builtin_dump_ui);
    globals.GetAdd(PyValue("wait_ready")) = PyValue::Function("wait_ready", builtin_wait_ready);
    globals.GetAdd(PyValue("wait_time")) = PyValue::Function("wait_time", builtin_wait_time);
    globals.GetAdd(PyValue("mock_ai")) = PyValue::Function("mock_ai", builtin_mock_ai);
    
    globals.GetAdd(PyValue("AutomationElement")) = PyValue::Function("AutomationElement", AutomationElement_Ctor);
    PyValue& current_class_dict = PyAutomationElement::GetClassDict();
    RegisterFunction(current_class_dict, "click", AutomationElement_click);
    RegisterFunction(current_class_dict, "set", AutomationElement_set);
}

END_UPP_NAMESPACE