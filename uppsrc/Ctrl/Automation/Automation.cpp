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
    ::Upp::String path = self.path;
    bool include_hidden = !self.visible;
    
    auto action = [path, include_hidden] {
        GuiAutomationVisitor vis;
        vis.include_hidden = include_hidden;
        ::Upp::Vector<Ctrl *> top = Ctrl::GetTopCtrls();
        for(Ctrl *c : top) {
            if(c->IsVisible() || include_hidden) {
                if(vis.Write(*c, path, ::Upp::Value(), true))
                    return true;
            }
        }
        return false;
    };

    if(Thread::IsMain()) {
        action();
    } else {
        PostCallback([=] { action(); });
    }
    
    return PyValue(true);
}

static PyValue AutomationElement_set(const Vector<PyValue>& args, void*) {
    PY_SELF(AutomationElement);
    if(args.GetCount() < 2) return PyValue(false);
    ::Upp::Value val = args[1].ToValue();
    ::Upp::String path = self.path;
    bool include_hidden = !self.visible;
    
    auto action = [path, include_hidden, val] {
        GuiAutomationVisitor vis;
        vis.include_hidden = include_hidden;
        ::Upp::Vector<Ctrl *> top = Ctrl::GetTopCtrls();
        for(Ctrl *c : top) {
            if(c->IsVisible() || include_hidden) {
                if(vis.Write(*c, path, val, false))
                    return true;
            }
        }
        return false;
    };

    if(Thread::IsMain()) {
        action();
    } else {
        PostCallback([=] { action(); });
    }
    
    return PyValue(true);
}

static PyValue builtin_find(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 1) return PyValue::None();
    String path = args[0].ToString();
    bool include_hidden = args.GetCount() >= 2 ? args[1].IsTrue() : false;
    
    GuiLock __;
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
    GuiLock __;
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
    // In threaded mode, just sleep a bit to let GUI process
    Sleep(100);
    return PyValue::None();
}

static PyValue builtin_wait_time(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 1) return PyValue::None();
    int ms = (int)(args[0].AsDouble() * 1000.0);
    Sleep(ms);
    return PyValue::None();
}

static AddMockFn sAddMock = nullptr;
void SetAddMockFn(AddMockFn fn) { sAddMock = fn; }

static EvalHook sEvalHook = nullptr;
static NavigateHook sNavigateHook = nullptr;
void SetAutomationHooks(EvalHook eval, NavigateHook navigate) {
    sEvalHook = eval;
    sNavigateHook = navigate;
}

static PyValue builtin_eval(const Vector<PyValue>& args, void*) {
    if(!sEvalHook || args.GetCount() < 1) return PyValue::None();
    try {
        return PyValue::FromValue(sEvalHook(args[0].ToString()));
    } catch(const std::exception& e) {
        return PyValue(String("Error: ") + e.what());
    }
}

static PyValue builtin_navigate(const Vector<PyValue>& args, void*) {
    if(!sNavigateHook || args.GetCount() < 1) return PyValue::None();
    try {
        sNavigateHook(args[0].ToString());
    } catch(...) {}
    return PyValue::None();
}

// Event System
static PyVM* sCurrentVM = nullptr;
static VectorMap<String, Vector<PyValue>> sEventHandlers;

void SetCurrentVM(PyVM* vm) {
	sCurrentVM = vm;
	sEventHandlers.Clear();
}

static PyValue builtin_bind_event(const Vector<PyValue>& args, void*) {
	if(args.GetCount() < 2) return PyValue::None();
	String event = args[0].ToString();
	PyValue callback = args[1];
	if(!callback.IsFunction() && !callback.IsBoundMethod()) return PyValue::None(); // Basic check
	
	sEventHandlers.GetAdd(event).Add(callback);
	return PyValue::None();
}

void TriggerEvent(const String& event) {
	int q = sEventHandlers.Find(event);
	if(q >= 0 && sCurrentVM) {
		const Vector<PyValue>& handlers = sEventHandlers[q];
		for(const PyValue& h : handlers) {
			PyVM temp_vm;
			const auto& src_globals = sCurrentVM->GetGlobals();
			auto& dst_globals = temp_vm.GetGlobals();
			for(int i = 0; i < src_globals.GetCount(); i++)
				dst_globals.Add(src_globals.GetKey(i), src_globals[i]);
			
			Vector<PyIR> ir;
			ir.Add(PyIR(PY_LOAD_CONST, h));
			ir.Add(PyIR(PY_CALL_FUNCTION, 0, 0));
			ir.Add(PyIR(PY_POP_TOP));
			ir.Add(PyIR(PY_LOAD_CONST, PyValue::None()));
			ir.Add(PyIR(PY_RETURN_VALUE));
			
			try {
				temp_vm.SetIR(ir);
				temp_vm.Run();
			} catch(const Exc& e) {
				Cout() << "Error in event handler '" << event << "': " << e << "\n";
			}
		}
	}
}

static PyValue builtin_mock_ai(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 2) return PyValue::None();
    String regex = args[0].ToString();
    String response = args[1].ToString();
    if(sAddMock) sAddMock(regex, response);
    return PyValue::None();
}

static PyValue builtin_exit(const Vector<PyValue>& args, void*) {
    int code = args.GetCount() >= 1 ? (int)args[0].AsInt64() : 0;
    _exit(code);
    return PyValue::None();
}

static PyValue builtin_log(const Vector<PyValue>& args, void*) {
    for(int i = 0; i < args.GetCount(); i++) {
        if(i) Cout() << " ";
        Cout() << args[i].ToString();
    }
    Cout() << "\n";
    Cout().Flush();
    return PyValue::None();
}

static PyValue builtin_send_key(const Vector<PyValue>& args, void*) {
    if(args.GetCount() < 1) return PyValue::None();
    dword key = (dword)args[0].AsInt64();
    Vector<Ctrl*> top = Ctrl::GetTopCtrls();
    for(Ctrl *c : top) {
        if(c->IsVisible() && c->IsOpen()) {
            c->Key(key, 1);
        }
    }
    return PyValue::None();
}

static PyValue builtin_find_all(const Vector<PyValue>& args, void*) {
    String parent_path = "";
    bool include_hidden = false;
    if(args.GetCount() >= 1) parent_path = args[0].ToString();
    if(args.GetCount() >= 2) include_hidden = args[1].IsTrue();
    
    GuiLock __;
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
    Cout() << "Registering Automation Bindings...\n";
    Cout().Flush();
    
    globals.GetAdd(PyValue("find")) = PyValue::Function("find", builtin_find);
    globals.GetAdd(PyValue("find_all")) = PyValue::Function("find_all", builtin_find_all);
    globals.GetAdd(PyValue("dump_ui")) = PyValue::Function("dump_ui", builtin_dump_ui);
    globals.GetAdd(PyValue("wait_ready")) = PyValue::Function("wait_ready", builtin_wait_ready);
    globals.GetAdd(PyValue("wait_time")) = PyValue::Function("wait_time", builtin_wait_time);
    globals.GetAdd(PyValue("mock_ai")) = PyValue::Function("mock_ai", builtin_mock_ai);
    globals.GetAdd(PyValue("eval")) = PyValue::Function("eval", builtin_eval);
    globals.GetAdd(PyValue("navigate")) = PyValue::Function("navigate", builtin_navigate);
    globals.GetAdd(PyValue("bind_event")) = PyValue::Function("bind_event", builtin_bind_event);
    globals.GetAdd(PyValue("send_key")) = PyValue::Function("send_key", builtin_send_key);
    globals.GetAdd(PyValue("log")) = PyValue::Function("log", builtin_log);
    globals.GetAdd(PyValue("_exit")) = PyValue::Function("_exit", builtin_exit);
    globals.GetAdd(PyValue("exit")) = PyValue::Function("exit", builtin_exit);
    
    // Key constants
    globals.GetAdd(PyValue("K_F5")) = PyValue((int64)K_F5);
    globals.GetAdd(PyValue("K_SHIFT_F5")) = PyValue((int64)K_SHIFT_F5);
    globals.GetAdd(PyValue("K_CTRL_SHIFT_F5")) = PyValue((int64)(K_CTRL | K_SHIFT | K_F5));
    globals.GetAdd(PyValue("K_ALT")) = PyValue((int64)K_ALT);
    globals.GetAdd(PyValue("K_CTRL")) = PyValue((int64)K_CTRL);
    globals.GetAdd(PyValue("K_SHIFT")) = PyValue((int64)K_SHIFT);
    globals.GetAdd(PyValue("K_ESCAPE")) = PyValue((int64)K_ESCAPE);
    globals.GetAdd(PyValue("K_ENTER")) = PyValue((int64)K_ENTER);
    
    for(int i = 0; i < 12; i++) {
        String name = "K_F";
        name << (i + 1);
        globals.GetAdd(PyValue(name)) = PyValue((int64)(K_F1 + i));
    }
    
    for(int i = 0; i < 26; i++) {
        String name = "K_";
        name << (char)('A' + i);
        globals.GetAdd(PyValue(name)) = PyValue((int64)('A' + i));
    }
    
    globals.GetAdd(PyValue("AutomationElement")) = PyValue::Function("AutomationElement", AutomationElement_Ctor);
    PyValue& current_class_dict = PyAutomationElement::GetClassDict();
    RegisterFunction(current_class_dict, "click", AutomationElement_click);
    RegisterFunction(current_class_dict, "set", AutomationElement_set);
}

END_UPP_NAMESPACE
